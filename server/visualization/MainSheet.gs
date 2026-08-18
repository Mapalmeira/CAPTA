class MainSheet extends Sheet {
  constructor() {
    super("Main");
  }

  getVoltage() {
    const volts = Number(this.getValue("C2"));

    if (!volts) {
      throw new Error("Error: set the voltage.");
    }

    return volts;
  }

  getDay() {
    const day = this.getValue("C3");

    if (!day || isNaN(day.getTime())) {
      throw new Error("Error: invalid date parameters.");
    }

    return day;
  }

  getStartHour() {
    const text = this.getValue("C4");
    const startHour = parseInt(text);

    if (isNaN(startHour)) {
      throw new Error("Error: invalid hour parameters.");
    }

    return startHour;
  }

  getEndHour() {
    const text = this.getValue("C5");
    const endHour = parseInt(text);

    if (isNaN(endHour)) {
      throw new Error("Error: invalid hour parameters.");
    }

    return endHour;
  }
}
