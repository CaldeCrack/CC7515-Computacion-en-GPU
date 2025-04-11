### Estudiante: Andrés Calderón Guardia

## Instrucciones de ejecución

1. Descomprimir tarea.
2. Ejecutar `cmake -S . -B build`.
3. Crear archivos binarios usando `cmake --build build`.
4. Para testear interacción en consola usar `./build/src/main`.
5. Para ejecutar archivo con tests escribir en la terminal `./build/test/tests`.

## Preguntas

1. ¿Afectaría en algo el tipo de dato de su matriz?, ¿Qué pasa si realiza operaciones de multiplicación con tipo de dato integer en vez de double?

Sí, el tipo de dato de la matriz afecta directamente los resultados de las operaciones, en particular si se utiliza `int` en vez de `double` no se podrían tener valores decimales, limitándose a tan solo valores enteros. La única ventaja que se tendría sería la precisión ya que todas las operaciones serían exactas bajo este contexto, exceptuando casos en que el valor se desborda de los límites del tipo `int`.

2. Si se empezaran a usar números muy pequeños o muy grandes y principalmente números primos, ¿Qué ocurre en términos de precisión?

En dicho caso sería posible empezar a perder precisión, en especial al realizar varias operaciones consecutivas por la acumulación de errores, dada la distribución de números que el tipo `double` es capaz de representar, ya que a medida que se aleja del 0 la densidad de estos disminuye. En particular, que se utilicen números primos no afectaría notablemente ya que siguen siendo números enteros.

3. ¿Pueden haber problemas de precisión si se comparan dos matrices idénticas pero con diferente tipo? (Matrix p1 == p2).

Sí, pueden haber problemas si las matrices fueron calculadas con diferente tipo de dato, por ejemplo una con `float` y otra con `double`, puesto que dado el nivel de precisión difiere entre ambos tipos se podría tener un número que no es posible de representar de forma exacta con el tipo `float` pero si con `double`, generando un caso en que por dicho valor se consideren que las 2 matrices sean diferentes.
