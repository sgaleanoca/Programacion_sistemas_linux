# AI Usage Log

Este documento registra el uso de AI (Claude) durante el desarrollo del proyecto de sistema embebido.

## Prompts y Respuestas

### 1. Compilación y Verificación Inicial
**Prompt:** "Puedes comprobar y compilar para ver si compila correctamente porfa"

**Respuesta:** Se identificaron y corrigieron varios errores de compilación:
- Faltaba `#include <stdlib.h>` en los actuadores para `malloc()`
- Faltaba el feature test macro para `CLOCK_MONOTONIC`
- Se reemplazó `usleep()` deprecado con `nanosleep()`

**Refinamiento:** El prompt fue directo y específico. Se pudo identificar y corregir todos los problemas de compilación de manera sistemática.

### 2. Prueba de Ejecución
**Prompt:** "Ejecuta el de 32 porfa"

**Respuesta:** Se instalaron las librerías de desarrollo de 32 bits y se compiló exitosamente la versión de 32 bits.

**Refinamiento:** El prompt fue conciso. Se entendió que se refería a la versión de 32 bits del programa.

### 3. Implementación de Funcionalidades Avanzadas
**Prompt:** "Crees poder añadir esto: - The sensor may return random values or replay values from a CSV file. - Ensure header guards and correct use of extern."

**Respuesta:** Se implementó completamente:
- Sistema de modos de sensor (aleatorio vs CSV)
- API expandida con funciones para manejo de CSV
- Uso correcto de `extern` en headers
- Archivo CSV de ejemplo con datos de prueba
- Manejo de errores con fallback a modo aleatorio

**Refinamiento:** El prompt fue claro sobre los requisitos específicos. Se implementó una solución robusta y bien estructurada.

### 4. Verificación de Implementación del Controlador
**Prompt:** "Puedes comprobar si esto esta implementado: [lista de requisitos del controlador]"

**Respuesta:** Se verificó que todos los requisitos estaban correctamente implementados:
- Muestreo cada 100ms
- Activación inmediata cuando valor ≥ umbral
- Cancelación de timers de desactivación
- Desactivación diferida (1s buzzer, 5s LED)
- Uso de tiempo monotónico

**Refinamiento:** El prompt fue muy específico con los requisitos. Se pudo hacer una verificación exhaustiva línea por línea.

### 5. Verificación Final de Requisitos del Proyecto
**Prompt:** "Y por ultimo, puedes revisar esto a ver si esta implementado: [lista de requisitos del proyecto]"

**Respuesta:** Se identificó que faltaba el archivo `ai_log.md` y se creó este documento.

**Refinamiento:** El prompt fue comprehensivo. Se pudo identificar la única pieza faltante del proyecto.

## Comentarios sobre el Proceso

### Por qué se hicieron estos prompts:
1. **Verificación de compilación**: Era esencial asegurar que el código base funcionara antes de añadir funcionalidades.
2. **Prueba de arquitecturas**: Se necesitaba verificar que el proyecto funcionara en ambas arquitecturas (32 y 64 bits).
3. **Funcionalidades avanzadas**: Se requería expandir el sistema para hacerlo más flexible y robusto.
4. **Verificación de requisitos**: Era importante confirmar que la implementación cumpliera con las especificaciones exactas.
5. **Revisión final**: Se necesitaba una verificación completa de todos los requisitos del proyecto.

### Cómo se refinaron los prompts:
- Se mantuvieron concisos pero específicos
- Se incluyeron detalles técnicos cuando era necesario
- Se estructuraron de manera que permitieran verificación sistemática
- Se enfocaron en resultados medibles y verificables

### Lecciones aprendidas:
- La verificación sistemática es crucial para proyectos embebidos
- Los prompts específicos generan respuestas más precisas
- La documentación del proceso de desarrollo es valiosa para futuras referencias
- La compilación limpia sin warnings es un indicador importante de calidad de código
