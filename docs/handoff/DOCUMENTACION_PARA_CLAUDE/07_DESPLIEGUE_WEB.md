# Publicar la WEB para tener un link que se toca y abre

## Sitio ya preparado
`SITIO_LISTO_PARA_DESPLEGAR/index.html` contiene la web completa autocontenida. `vercel.json` y `netlify.toml` están listos para un hosting estático. El HTML **no** debe entregarse como enlace de visualización de código, debe alojarse con `Content-Type: text/html`.

## Opciones
1. **Vercel:** con cuenta conectada y autorización, crear proyecto, seleccionar como raíz `SITIO_LISTO_PARA_DESPLEGAR`, desplegar estático; verificar URL real con HTTPS.
2. **Netlify:** publicar la misma carpeta. Para drag-and-drop, comprimir SOLO el contenido de `SITIO_LISTO_PARA_DESPLEGAR/` con `index.html` en la raíz.
3. **GitHub Pages:** publicar un repo público autorizado desde carpeta raíz o docs con `index.html`, habilitar Pages y comprobar URL real.

## Smoke test obligatorio
- Ir a la URL compartible desde un navegador fuera de ChatGPT; comprobar que aparece la interfaz y que el servidor entrega `text/html`.
- Generar patrón; pulsar Play/Stop (después de gesto); exportar MIDI y WAV; abrir ficheros; guardar y recuperar JSON.
- Verificar consola JS y red sin errores; probar móvil. Web MIDI requiere Chrome/Edge compatible, permisos y dispositivo conectado; en móvil la compatibilidad es limitada.

## Una URL no se inventa
`dfd-acid-engine.vercel.app` es una idea de subdominio, NO garantiza disponibilidad ni despliegue. Reportar sólo la URL devuelta por el proveedor una vez publicada y comprobada.
