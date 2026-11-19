#include "PluginEditor.h"
#include "BinaryData.h"
using Completion = juce::WebBrowserComponent::NativeFunctionCompletion;

DfdEditor::DfdEditor(DfdProcessor& p) : AudioProcessorEditor(p), proc(p) {
  web = std::make_unique<juce::WebBrowserComponent>(makeOptions());
  addAndMakeVisible(*web);
  setResizable(true, true);
  setResizeLimits(900, 560, 2800, 1800);
  setSize(1440, 880);
  web->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
  startTimerHz(30);
}
DfdEditor::~DfdEditor() { stopTimer(); }
void DfdEditor::resized() { web->setBounds(getLocalBounds()); }

std::optional<juce::WebBrowserComponent::Resource> DfdEditor::getResource(const juce::String& url) {
  if (url == "/" || url.startsWith("/index.html")) {
    std::vector<std::byte> data((size_t)BinaryData::index_htmlSize);
    std::memcpy(data.data(), BinaryData::index_html, data.size());
    return juce::WebBrowserComponent::Resource { std::move(data), "text/html; charset=utf-8" };
  }
  return std::nullopt;
}

static juce::File homeDir() {
  auto d = juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("DFD ACID ENGINE");
  d.createDirectory(); return d;
}

juce::WebBrowserComponent::Options DfdEditor::makeOptions() {
  auto o = juce::WebBrowserComponent::Options {}
    .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
    .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2 {}.withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("DFD ACID ENGINE WebView")))
    .withNativeIntegrationEnabled()
    .withKeepPageLoadedWhenBrowserIsHidden()
    .withResourceProvider([this](const auto& url) { return getResource(url); })
    .withNativeFunction("msg", [this](const juce::Array<juce::var>& a, Completion done) { if (a.size() > 0) proc.message(juce::JSON::parse(a[0].toString())); done(juce::var()); })
    .withNativeFunction("getProject", [this](const juce::Array<juce::var>&, Completion done) { done(proc.projectForPage()); })
    .withNativeFunction("setUi", [this](const juce::Array<juce::var>& a, Completion done) { if (a.size() > 0) proc.setUi(a[0].toString()); done(juce::var()); })
    .withNativeFunction("openFile", [this](const juce::Array<juce::var>& a, Completion done) {
      chooser = std::make_unique<juce::FileChooser>("Open", homeDir(), a.size() > 0 ? a[0].toString() : "*.json");
      chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [done](const juce::FileChooser& fc) {
        auto f = fc.getResult(); if (!f.existsAsFile()) { done(juce::var()); return; }
        juce::DynamicObject::Ptr r = new juce::DynamicObject(); r->setProperty("name", f.getFileName()); r->setProperty("text", f.loadFileAsString()); done(juce::var(r.get())); }); })
    .withNativeFunction("saveFile", [this](const juce::Array<juce::var>& a, Completion done) {
      // a[0] file name, a[1] base64 contents: a save window, starting in ~/Music/DFD ACID ENGINE
      if (a.size() < 2) { done(false); return; }
      auto bytes = std::make_shared<juce::MemoryOutputStream>(); juce::Base64::convertFromBase64(*bytes, a[1].toString());
      chooser = std::make_unique<juce::FileChooser>("Save", homeDir().getChildFile(juce::File::createLegalFileName(a[0].toString())), "*" + juce::File(a[0].toString()).getFileExtension());
      chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [bytes, done](const juce::FileChooser& fc) {
        auto f = fc.getResult(); const bool ok = f != juce::File() && f.replaceWithData(bytes->getData(), bytes->getDataSize());
        if (ok) f.revealToUser(); done(ok); }); })
    .withNativeFunction("dragMidi", [this](const juce::Array<juce::var>& a, Completion done) {
      // a[0] file name, a[1] base64 .mid: written to a temporary folder and dragged out of the plug-in onto a DAW track
      if (a.size() < 2) { done(false); return; }
      juce::MemoryOutputStream mo; juce::Base64::convertFromBase64(mo, a[1].toString());
      auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("DFD ACID ENGINE MIDI"); dir.createDirectory();
      auto f = dir.getChildFile(juce::File::createLegalFileName(a[0].toString()));
      const bool ok = f.replaceWithData(mo.getData(), mo.getDataSize());
      if (ok) juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
      done(ok); })
    .withNativeFunction("openUrl", [](const juce::Array<juce::var>& a, Completion done) {
      // only the donation page, the project's pages and Polaris Sound Lab open outside the plug-in
      auto url = a.size() > 0 ? a[0].toString() : juce::String();
      if (url.startsWith("https://www.paypal.com/donate/") || url.startsWith("https://github.com/drophlex/") || url.startsWith("https://drophlex.github.io/")
          || url.startsWith("https://www.instagram.com/polaris.sound.lab") || url.startsWith("https://www.youtube.com/@PolarisSoundLab"))
        juce::URL(url).launchInDefaultBrowser();
      done(juce::var()); });
  return o;
}

// telemetry -> page, the same shape the web worklet posts; parameters every few frames so DAW automation shows on the page
void DfdEditor::timerCallback() {
  int w = 0; proc.snapshot(tele, scope, w, params);
  juce::DynamicObject::Ptr o = new juce::DynamicObject();
  juce::Array<juce::var> t, s; for (float v : tele) t.add(v); for (float v : scope) s.add(v);
  o->setProperty("tele", t); o->setProperty("scope", s); o->setProperty("w", w);
  if (++frame % 4 == 0) { juce::Array<juce::var> p; for (float v : params) p.add(v); o->setProperty("params", p); }
  web->emitEventIfBrowserIsVisible("tele", juce::var(o.get()));
}
