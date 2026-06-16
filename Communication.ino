void sendBootMessage() {
  StaticJsonDocument<160> bootData;

  bootData["evt"] = "boot";
  bootData["fw_version"] = FW_VERSION;

  serializeJson(bootData, Serial);
  Serial.println();
}

void sendAck(long sequence) {
  StaticJsonDocument<64> ackData;

  ackData["evt"] = "ack";
  ackData["seq"] = sequence;

  serializeJson(ackData, Serial);
  Serial.println();
}

void processSerialCommunication(unsigned long now) {
  if (Serial.available() <= 0) {
    return;
  }

  // JSON 명령 처리
  if (Serial.peek() == '{') {
    StaticJsonDocument<128> commandData;

    DeserializationError error = deserializeJson(commandData, Serial);
    if (error) {
      return;
    }

    lastCommandTime = now;
    pcTimeout = false;

    String command = commandData["cmd"] | "";
    long sequence = commandData["seq"] | 0;

    if (command == "start") {
      systemRunning = 1;
      sendAck(sequence);
    }
    else if (command == "stop") {
      systemRunning = 0;
      sendAck(sequence);
    }
    else if (command == "motor") {
      int motorValue = commandData["value"] | 0;
      systemRunning = motorValue > 0 ? 1 : 0;
      sendAck(sequence);
    }
    else if (command == "servo") {
      String target = commandData["target"] | "A";
      int angle = commandData["value"] | SERVO_ACTION_ANGLE;

      if (target == "B") {
        addServoTask(2, angle);
      }
      else {
        addServoTask(1, angle);
      }

      sendAck(sequence);
    }
    else if (command == "ping") {
      sendAck(sequence);
    }

    return;
  }

  // 간단한 문자 명령 처리
  char command = Serial.read();

  if (command == '0') {
    systemRunning = 0;
    lastCommandTime = now;
    pcTimeout = false;
  }
  else if (command == '1') {
    systemRunning = 1;
    lastCommandTime = now;
    pcTimeout = false;
  }
  else if (command == 'A') {
    addServoTask(1, SERVO_ACTION_ANGLE);
    lastCommandTime = now;
    pcTimeout = false;
  }
  else if (command == 'B') {
    addServoTask(2, SERVO_ACTION_ANGLE);
    lastCommandTime = now;
    pcTimeout = false;
  }
}

void sendHeartbeat(unsigned long now) {
  if (now - lastHeartbeatTime < HEARTBEAT_INTERVAL_MS) {
    return;
  }

  StaticJsonDocument<128> heartbeatData;

  heartbeatData["evt"] = "hb";
  heartbeatData["uptime"] = now;
  heartbeatData["seq"] = ++sequenceNumber;

  serializeJson(heartbeatData, Serial);
  Serial.println();

  lastHeartbeatTime = now;
}

void sendSensorStatus(unsigned long now) {
  if (now - lastSensorSendTime < SENSOR_SEND_INTERVAL_MS) {
    return;
  }

  float measuredTemperature = dht.readTemperature();

  if (!isnan(measuredTemperature)) {
    temperatureValue = measuredTemperature;
  }

  StaticJsonDocument<160> sensorData;

  sensorData["evt"] = "sensor";
  sensorData["st"] = systemRunning;
  sensorData["t"] = temperatureValue;
  sensorData["s6"] = objectDetected ? 1 : 0;
  sensorData["seq"] = ++sequenceNumber;

  serializeJson(sensorData, Serial);
  Serial.println();

  lastSensorSendTime = now;
}

void processPcTimeout(unsigned long now) {
  if (now - lastCommandTime > PC_TIMEOUT_MS) {
    systemRunning = 0;
    pcTimeout = true;
  }
}
