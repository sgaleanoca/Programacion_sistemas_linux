## Iniciarlo manualmente:

**1. Compila el proyecto:**

    make

**2. Copia el ejecutable a una ruta del sistema:**

    sudo cp build/assignment-sensor /usr/local/bin/

**3. Copia el archivo del servicio al directorio de systemd:**

    sudo cp systemd/assignment-sensor.service /etc/systemd/system/

**4. Recarga el demonio de systemd para que lea el nuevo archivo:**

    sudo systemctl daemon-reload

**5. Habilita e inicia el servicio en un solo paso:**

    sudo systemctl enable --now assignment-sensor.service

enable lo hace arrancar con el sistema.

--now lo inicia inmediatamente.

**6. Verifica su estado:**

    systemctl status assignment-sensor.service

Aquí verás si está active (running) y las últimas líneas de su log.

**7. Mira el log en tiempo real:**

    tail -f /tmp/sensor_log.txt

**8. Para detener el servicio:**

    sudo systemctl stop assignment-sensor.service

Al hacer esto, systemd enviará la señal SIGTERM a tu programa, que la capturará, saldrá del bucle y cerrará el archivo de log limpiamente.

**9. Para deshabilitar el servicio (para que no inicie con el sistema):**

    sudo systemctl disable assignment-sensor.service


## Ejecutar los tests:

**Para test.sh:**

    sudo ./tests/test.sh

**Para demo_realtime.sh:**

    sudo systemctl start assignment-sensor.service

    sudo ./tests/demo_realtime.sh

