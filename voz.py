
import time
import unicodedata

import serial #type: ignore
import sounddevice as sd #type: ignore
import speech_recognition as sr #type: ignore


# ==========================================
# CONFIGURACIÓN
# ==========================================

PUERTO_ARDUINO = "COM3"
BAUDRATE = 115200

FS = 16000
DURACION = 3

recognizer = sr.Recognizer()

PALABRAS_ABRIR = {"abrir", "abre", "abri"}
PALABRAS_CERRAR = {"cerrar", "cierra", "cerra"}
PALABRAS_ARRIBA = {"arriba", "subir", "sube", "subi"}
PALABRAS_ABAJO = {"abajo", "bajar", "baja"}


# ==========================================
# NORMALIZAR TEXTO
# ==========================================

def normalizar(texto):

    texto = texto.lower()

    texto = "".join(
        c
        for c in unicodedata.normalize("NFD", texto)
        if unicodedata.category(c) != "Mn"
    )

    return texto.strip()


# ==========================================
# RECONOCIMIENTO DE VOZ
# ==========================================

def escuchar():

    print("\n🎤 Habla ahora...")

    try:
        audio = sd.rec(
            int(DURACION * FS),
            samplerate=FS,
            channels=1,
            dtype="int16"
        )

        sd.wait()

        audio_data = sr.AudioData(
            audio.flatten().tobytes(),
            FS,
            2
        )

        texto = recognizer.recognize_google(
            audio_data,
            language="es-AR"
        )

        return normalizar(texto)

    except sr.UnknownValueError:
        print("❌ No pude entenderte")
        return None

    except sr.RequestError as e:
        print("❌ Error de reconocimiento:", e)
        return None

    except sd.PortAudioError as e:
        print("❌ Error del micrófono:", e)
        return None


# ==========================================
# DETECTAR COMANDO
# ==========================================

def detectar_comando(texto):

    if not texto:
        return None

    palabras = set(texto.split())

    if palabras & {"pinza", "brazo"}:
        if palabras & PALABRAS_CERRAR:
            return "PINZA,CERRAR"
        if palabras & PALABRAS_ABRIR:
            return "PINZA,ABRIR"

        return None

    if "base" in palabras:
        if "izquierda" in palabras:
            return "BASE,IZQUIERDA"
        if "derecha" in palabras:
            return "BASE,DERECHA"
        return None

    for articulacion in ("HOMBRO", "CODO"):
        if articulacion.lower() in palabras:
            if palabras & PALABRAS_ARRIBA:
                return f"{articulacion},ARRIBA"
            if palabras & PALABRAS_ABAJO:
                return f"{articulacion},ABAJO"
            return None

    return None


# ==========================================
# ENVIAR COMANDO POR SERIAL
# ==========================================

def enviar_comando(arduino, comando):

    mensaje = comando + "\n"

    arduino.reset_input_buffer()

    arduino.write(mensaje.encode("utf-8"))
    arduino.flush()

    print("📤 Enviado:", comando)

    # Esperar confirmación del Arduino
    limite = time.monotonic() + 2

    while time.monotonic() < limite:

        respuesta = arduino.readline().decode(
            "utf-8", errors="replace"
        ).strip()

        if not respuesta:
            continue

        print("📥 Arduino:", respuesta)

        if respuesta.startswith(("OK:", "ERROR:")):
            return

    print("⚠️ No se recibió confirmación del Arduino")


# ==========================================
# PROGRAMA PRINCIPAL
# ==========================================

def main():

    try:
        arduino = serial.Serial(
            PUERTO_ARDUINO,
            BAUDRATE,
            timeout=0.5
        )

    except serial.SerialException as e:
        print("❌ No se pudo conectar al Arduino:", e)
        return

    try:
        time.sleep(2)

        print("✅ Arduino conectado en", PUERTO_ARDUINO)
        print("🎙️ Control por voz iniciado")
        print("Presiona CTRL+C para salir")

        while True:

            input("\nPresiona ENTER para hablar...")

            texto = escuchar()

            if texto is None:
                continue

            print("📝 Reconocido:", texto)

            comando = detectar_comando(texto)

            if comando is None:
                print("❌ Comando no válido")
                continue

            print("🤖 Comando detectado:", comando)

            enviar_comando(arduino, comando)

    except KeyboardInterrupt:
        print("\n🛑 Programa finalizado")

    finally:
        arduino.close()
        print("Puerto serial cerrado")


if __name__ == "__main__":
    main()
