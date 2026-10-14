
#include <Servo.h>

// ==========================================
// INVERSIÓN DE EJES (JOYSTICKS Y VOZ)
// ==========================================

const bool invertBase     = true;
const bool invertShoulder = false;
const bool invertElbow    = true;
const bool invertGripper  = false;

// ==========================================
// SERVOS
// ==========================================

Servo servoBase;
Servo servoShoulder;
Servo servoElbow;
Servo servoGripper;

// ==========================================
// PINES
// ==========================================

const int PinTestigo = 3;

const int basePin     = 11;
const int shoulderPin = 9;
const int elbowPin    = 10;
const int gripperPin  = 6;

// Joysticks
const int xdirPin = A0;
const int ydirPin = A1;
const int zdirPin = A2;
const int gdirPin = A3;

// ==========================================
// CONTROL
// ==========================================

const int movementDelay = 10;
const int stepPerTick = 1;
const int voiceStep = 15;
const int deadzone = 80;

const unsigned long detachDelay = 1000;

// ==========================================
// LÍMITES
// ==========================================

const int minBaseAngle = 0;
const int maxBaseAngle = 170;

const int minShoulderAngle = 115;
const int maxShoulderAngle = 180;

const int minElbowAngle = 0;
const int maxElbowAngle = 130;

const int minGripperAngle = 130;
const int maxGripperAngle = 180;

// ==========================================
// PINZA (CORREGIDA)
// ==========================================

// Ahora está invertida respecto al código anterior

const int gripperOpenAngle = 180;
const int gripperClosedAngle = 130;

// ==========================================
// ESTADO
// ==========================================

int baseAngle = (minBaseAngle + maxBaseAngle) / 2;
int shoulderAngle = minShoulderAngle;
int elbowAngle = minElbowAngle;

// Mantener el ángulo inicial del código original
int gripperAngle = minGripperAngle;

unsigned long lastMovement = 0;
bool servosAttached = false;

// ==========================================
// JOYSTICK
// ==========================================

int getDirectionFromJoystick(int analogValue) {

  if (analogValue < (512 - deadzone))
    return -1;

  if (analogValue > (512 + deadzone))
    return 1;

  return 0;
}

// ==========================================
// ATTACH
// ==========================================

void attachServos() {

  if (!servosAttached) {

    servoBase.attach(basePin);
    servoShoulder.attach(shoulderPin);
    servoElbow.attach(elbowPin);
    servoGripper.attach(gripperPin);

    servoBase.write(baseAngle);
    servoShoulder.write(shoulderAngle);
    servoElbow.write(elbowAngle);
    servoGripper.write(gripperAngle);

    servosAttached = true;
  }
}

// ==========================================
// DETACH
// ==========================================

void detachServos() {

  if (servosAttached) {

    servoBase.detach();
    servoShoulder.detach();
    servoElbow.detach();
    servoGripper.detach();

    servosAttached = false;
  }
}

// ==========================================
// MOVER SERVO
// ==========================================

void moveServo(
  Servo &servo,
  int &currentAngle,
  int direction,
  int degrees,
  int minAngle,
  int maxAngle,
  bool invertAxis
) {

  if (direction == 0)
    return;

  int delta =
    (invertAxis ? -direction : direction) * degrees;

  int newAngle = constrain(
    currentAngle + delta,
    minAngle,
    maxAngle
  );

  if (newAngle != currentAngle) {

    currentAngle = newAngle;
    servo.write(currentAngle);

    Serial.print("Nuevo angulo: ");
    Serial.println(currentAngle);
  }
}

// ==========================================
// PROCESAR COMANDO SERIAL
// ==========================================

void processCommand(String command) {

  command.trim();
  command.toUpperCase();

  if (command.length() == 0)
    return;

  bool validCommand = true;

  // BASE

  if (command == "BASE,IZQUIERDA") {

    attachServos();

    moveServo(
      servoBase, baseAngle, -1, voiceStep,
      minBaseAngle, maxBaseAngle, invertBase
    );
  }

  else if (command == "BASE,DERECHA") {

    attachServos();

    moveServo(
      servoBase, baseAngle, 1, voiceStep,
      minBaseAngle, maxBaseAngle, invertBase
    );
  }

  // HOMBRO

  else if (command == "HOMBRO,ARRIBA") {

    attachServos();

    moveServo(
      servoShoulder, shoulderAngle, 1, voiceStep,
      minShoulderAngle, maxShoulderAngle,
      invertShoulder
    );
  }

  else if (command == "HOMBRO,ABAJO") {

    attachServos();

    moveServo(
      servoShoulder, shoulderAngle, -1, voiceStep,
      minShoulderAngle, maxShoulderAngle,
      invertShoulder
    );
  }

  // CODO

  else if (command == "CODO,ARRIBA") {

    attachServos();

    moveServo(
      servoElbow, elbowAngle, 1, voiceStep,
      minElbowAngle, maxElbowAngle, invertElbow
    );
  }

  else if (command == "CODO,ABAJO") {

    attachServos();

    moveServo(
      servoElbow, elbowAngle, -1, voiceStep,
      minElbowAngle, maxElbowAngle, invertElbow
    );
  }

  // PINZA (CORREGIDA)

  else if (command == "PINZA,ABRIR") {

    attachServos();

    gripperAngle = gripperOpenAngle;
    servoGripper.write(gripperAngle);
  }

  else if (command == "PINZA,CERRAR") {

    attachServos();

    gripperAngle = gripperClosedAngle;
    servoGripper.write(gripperAngle);
  }

  else {
    validCommand = false;
  }

  // RESULTADO

  if (validCommand) {

    lastMovement = millis();

    Serial.print("OK: ");
    Serial.println(command);
  }

  else {

    Serial.print("ERROR: Comando desconocido: ");
    Serial.println(command);
  }
}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  pinMode(PinTestigo, OUTPUT);
  digitalWrite(PinTestigo, HIGH);

  lastMovement = millis();

  Serial.println("Brazo listo");
}

// ==========================================
// LOOP
// ==========================================

void loop() {

  // ======================================
  // RECIBIR COMANDOS DESDE PYTHON
  // ======================================

  if (Serial.available()) {

    String command = Serial.readStringUntil('\n');

    processCommand(command);
  }

  // ======================================
  // LEER JOYSTICKS
  // ======================================

  int xDir = getDirectionFromJoystick(analogRead(xdirPin));
  int yDir = getDirectionFromJoystick(analogRead(ydirPin));
  int zDir = getDirectionFromJoystick(analogRead(zdirPin));
  int gDir = getDirectionFromJoystick(analogRead(gdirPin));

  bool moving =
    xDir != 0 ||
    yDir != 0 ||
    zDir != 0 ||
    gDir != 0;

  // ======================================
  // MOVIMIENTO CON JOYSTICKS
  // ======================================

  if (moving) {

    attachServos();

    lastMovement = millis();

    moveServo(
      servoBase, baseAngle, xDir, stepPerTick,
      minBaseAngle, maxBaseAngle, invertBase
    );

    moveServo(
      servoShoulder, shoulderAngle, yDir, stepPerTick,
      minShoulderAngle, maxShoulderAngle,
      invertShoulder
    );

    moveServo(
      servoElbow, elbowAngle, zDir, stepPerTick,
      minElbowAngle, maxElbowAngle, invertElbow
    );

    moveServo(
      servoGripper, gripperAngle, gDir, stepPerTick,
      minGripperAngle, maxGripperAngle,
      invertGripper
    );
  }

  // ======================================
  // DETACH AUTOMÁTICO
  // ======================================

  else if (
    servosAttached &&
    millis() - lastMovement >= detachDelay
  ) {

    detachServos();
  }

  delay(movementDelay);
}
