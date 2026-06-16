void addServoTask(int servoId, int angle) {
  int nextTail = (queueTail + 1) % SERVO_QUEUE_SIZE;

  // 명령 저장 공간이 가득 차면 새 명령을 추가하지 않음
  if (nextTail == queueHead) {
    return;
  }

  servoQueue[queueTail].servoId = servoId;
  servoQueue[queueTail].angle = constrain(angle, 0, 180);
  queueTail = nextTail;
}

void processServoControl(unsigned long now) {
  // 비상정지 상태에서는 서보를 원위치로 복귀하고 대기
  if (emergencyStopped) {
    moveBothServos(SERVO_WAIT_ANGLE);
    servoActive = false;
    queueHead = queueTail;
    return;
  }

  // 대기 중인 서보 명령 실행
  if (!servoActive && queueHead != queueTail) {
    ServoTask task = servoQueue[queueHead];
    queueHead = (queueHead + 1) % SERVO_QUEUE_SIZE;

    if (task.servoId == 1) {
      servo1.write(task.angle);
    }
    else {
      servo2.write(task.angle);
    }

    servoActive = true;
    servoStartTime = now;
  }

  // 일정 시간이 지나면 두 서보를 원위치로 복귀
  if (servoActive &&
      now - servoStartTime >= SERVO_RETURN_TIME_MS) {
    moveBothServos(SERVO_WAIT_ANGLE);
    servoActive = false;
  }
}

void moveBothServos(int angle) {
  servo1.write(angle);
  servo2.write(angle);
}
