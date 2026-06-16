void processEmergencyStop(unsigned long now) {
  emergencyStopped =
    digitalRead(EMERGENCY_BUTTON_PIN) == LOW;

  if (emergencyStopped) {
    // 0.5초 주기로 부저를 반복 출력
    if (now % 500 < 350) {
      tone(BUZZER_PIN, 2500);
    }
    else {
      noTone(BUZZER_PIN);
    }
  }
  else {
    noTone(BUZZER_PIN);
  }
}
