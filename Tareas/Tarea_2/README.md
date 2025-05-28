### Estudiante: Andrés Calderón Guardia

## Dispositivo
Utilicé

## Prerrequisitos
No estoy seguro si la tarea funcionará directamente para cualquier dispositivo, me costó la vida lograr que funcionara con OpenCL, supuestamente se deben tener todas las librerías necesarias instaladas tanto para CUDA como para OpenCL según el sistema operativo con el fin de que funcione.

## Instrucciones de ejecución

1. Descomprimir tarea.
2. Ejecutar `cmake -S . -B build`.
3. Crear archivos binarios usando `cmake --build build`.
4. Ejecución distintos binarios:

    a. Serial: `./build/src/serial`

    b. CUDA: `/build/src/cuda`

    c. OpenCL: `cd build/src && ./opencl` (por alguna razón no logré que utilizara el kernel copiado en `build/src` a menos que el usuario se mueva a esta carpeta).

5. Tras ejecutar cada binario se crea un archivo `.csv` con los resultados, en particular para Serial y CUDA se crean en la carpeta raíz, pero en OpenCL se crea en la carpeta `build/src`

## Gráficos

1. Se deben tener todos los archivos `.csv` en la misma carpeta que estos scripts, sin haber cambiado los nombres.
2. Usar ambiente virtual (opcional):
    
    a. Creación: `python3 -m venv .venv`.

    b. Activación: (Linux) `source .venv/bin/activate` (Windows) `.venv\Scripts\Activate` (macOS) `no sé`

3. Instalar librerías utilizadas: `pip install -r requirements.txt`.
4. Ejecutar los distintos scripts según los gráficos que se quieran obtener, estos se guardan en archivos `.png`.

