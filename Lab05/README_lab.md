# Lab05 - TCP RAW

## Modificaciones

- Renombre `tpc_session_drain_until_close()` a `tcp_session_receive_message()` para generalizar retorno de mensajes del servidor. No cambie significativamente el código, solo fue un cambio de nombre y un `return` cuando se lee un mensaje.
- Añadí un ciclo `while(1)` en la funcion `run_session()` para iterar sobre el proceso de enviar mensajes y leer respuesta del servidor.
- Incluí un nuevos archivos tcp_logs.c y tcp_logs.h para definir funciones de utilidad para hacer prints y registrar el flujo de envio y recibo de paquetes. Makefile tambien fue actualizado para compilar esta adición.
- Adición de logs para dividir secciones en tcp_client.c en el intercambio y logs especificos con banderas, números de secuencia y acknowledgment numbers para envio y recepción

## Interacción

La interacción con el usuario es bastante simple, iterando a traves de un ciclo que arrance después de haber hecho el handshake y establecido la conexión con el servidor. Un prompt aparece de la forma "Insertar mensaje >" para leer de la consola. El usuario inserta su mensaje y eso lleva a la parte de enviar el mensaje y proceder a escuchar inmediatamente por la respuesta. Esto se repite de forma constante hasta que el comando del usuario sea "EXIT" o "EXIT OFF", donde se rompe el ciclo y se llama a la función para cerrar la conexión. 

## Generación de Output

El output del programa se genera a través de los logs repartidos entre tcp_client.c y tcp_session.c. Dentro del primero se encuentras print de divisores que indican donde inicia y termina cada etapa de la comunicación, incluyendo handshake, intercambio de datos y cierre de la conexión. Los print colocados en el segundo abarcan los envios y recibos de paquetes en cada momento, colocados en las secciones del programa que envían paqutes de control, envían mensajes y leen los mensajes recibidos. 

## Información del programa

En la consola, el programa genera una serie de logs que registran cada paquete de control y datos enviado y recibido por el programa en su comunicación con el servidor TCP. Cada salida del programa muestra el intercambio en el handshake, en el cierre de la conexión y el intercambio de datos que ocurre entre ellos. Aquí se registran las bandres, números de secuencia y de ack de cada paquete enviado y recibido por el cliente. 

## Prueba con WireShark 

La verificación con WireShark se realizó utilizando sus filtros que ofrece el programa para identificar los paquetes que tenían un origen y destino la misma IP. Como ambos corren en mi propia máquina este filtro aseguraba que los paquetes revisados fueran los que se dan en la comunicación entre el cliente y servidor corriendo simultáneamente. 