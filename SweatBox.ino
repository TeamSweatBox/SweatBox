int steps_taken;
int goal;
int[] recent_steps;

void setup() {
  steps_taken = 0;
  goal = enter_goal();
  recent_steps = new int[10];
}

void loop() {
  if (step()) {
     steps_taken++;

  }

  int current_activity_level = get_activity_level();

}

boolean step() {
  // ask the accelerometer if we're stepping. Debounce ??
  return true; //TODO
}

int enter_goal() {
  //Enter the target step count.
}

class activity_meter {
  /* Keep up with when the most recent 10 steps occured
   * so that we can determine how active the user is.
   */

  LinkedList queue = new LinkedList(); // queue of most recent steps

  void add_step() {
    queue.offer(millis());
    if (queue.size() > 10) {
      queue.pop()
    }
  }

  int activity_level() {
    //TODO return a number related to step frequency
  }

}
