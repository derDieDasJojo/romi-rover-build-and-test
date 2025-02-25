#include "ArduinoUno.h"
#include "IncrementalEncoderUno.h"
#include "Average.h"

IncrementalEncoderUno left_encoder;
IncrementalEncoderUno right_encoder;
Average left_filter;
Average right_filter;

double left_target = 0.0;
double right_target = 0.0;
double left_speed = 0.0;
double right_speed = 0.0;
double left_signal = 0.0;
double right_signal = 0.0;

double max_acceleration = 0.2;

double slope_adjustment = 1.0;
double adjustment_factor = 0.001;

static const unsigned long kUpdateInterval = 20;

void update_left_encoder()
{
        left_encoder.update();
}

void update_right_encoder()
{
        right_encoder.update();
}



#define N 200
#define M 500
double left_recordings[N];
double right_recordings[N];
int recording_index = 0;
int counter = 0;

#if 0
double kEncoderSteps = 2100.0;
double Kp = 0.00003;
double Ki = 0.00001;

#else
double kEncoderSteps = 16000.0;
double Kp = 0.00003; // TODO
double Ki = 0.00001; // TODO
#endif

// delta_target = speed rev/s * encoder_steps steps/rev * dt s;
// delta_target = speed * 2100 * (20 / 1000);
// delta_target = vi * 42000 / 1000;
// delta_target = speed * 42;
// 16000*20/1000 = 320  
double kDeltaCoeff = kEncoderSteps * (double) kUpdateInterval / 1000.0;

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;

        init_timer1();
        
        left_encoder.init(kEncoderSteps, -1, 2, 4, update_left_encoder);
        right_encoder.init(kEncoderSteps, 1, 3, 5, update_right_encoder);

        randomSeed(analogRead(0));

        set_pulse_width(3000, 3000);
}

void init_timer1()
{
        // From https://wolles-elektronikkiste.de/en/timer-and-pwm-part-2-16-bit-timer1
        // Clear OC1A and OC1B on Compare Match / Set OC1A and OC1B at Bottom; 
        // Wave Form Generator: Fast PWM 14, Top = ICR1
        TCCR1A = (1 << COM1A1) + (1 << COM1B1) + (1 << WGM11); 
        TCCR1B = (1 << WGM13) + (1 << WGM12) + (1 << CS11); // prescaler = 8

        // F_CPU = 16000000
        // frequency = 50 Hz -> period = 20 ms
        // prescaler = 8
        // top = F_CPU / (prescaler * frequency) - 1 = 39999

        // 1 tick = 1 period / 40000 = 20 ms / 40000 s = 1/2 µs

        ICR1 = 39999;
        OCR1A = 3000; // 3000 ticks = 3000 * 1/2 µs = 1500 µs = 1.5 ms
        OCR1B = 3000;
        DDRB |= (1 << PB1) | (1 << PB2);
} 

static inline void set_pulse_width(uint16_t left, uint16_t right)
{
        OCR1A = left;
        OCR1B = right;
}

static inline void set_output(double left, double right)
{
        set_pulse_width((uint16_t) (3000.0 + 1000.0 * left),
                        (uint16_t) (3000.0 + 1000.0 * right)); 
}

static inline void set_target_speeds(double left, double right)
{
        left_target = left;
        right_target = right;
}

static inline void update_speeds(double dt)
{
        left_speed = update_speed(left_target, left_speed, dt);
        right_speed = update_speed(right_target, right_speed, dt);
}

static inline double update_speed(double target, double speed, double dt)
{
        if (speed < target) {
                speed += max_acceleration * dt;
                if (speed > target)
                        speed = target;
        } else if (speed > target) {
                speed -= max_acceleration * dt;
                if (speed < target)
                        speed = target;
        }
        return speed;
}


void foo()
{
        unsigned long last_time = 0;
        unsigned long now = millis();
        
        if (now - last_time >= kUpdateInterval) {
                double dt = (now - last_time) / 1000.0;

                //measure_signal_to_speed_curve(dt);
                //test_speed_to_signal_curve(0.6, dt);
                //test_speed_to_signal_curve_with_pid(0.6, dt);
                //test_speed_to_signal_curve_with_stepup_stepdown(0.6, dt);
                //test_speed_to_signal_curve_with_slope_adjustment(0.6, dt);

                // double instant_left_speed = left_encoder.get_speed(dt);
                // double instant_right_speed = right_encoder.get_speed(dt);
                // double e = left_speed - instant_left_speed; // normalized?
        
                // update_speeds(dt);
                
                // left_signal = estimate_signal(left_speed, 1.0);
                // right_signal = estimate_signal(right_speed, 1.0);


                // double epsilon = 0.00245;
                // if (left_speed > 0) {
                //         if (e > epsilon) {
                //                 slope_adjustment += adjustment_factor;
                //         } else if (e < -epsilon) {
                //                 slope_adjustment -= adjustment_factor;
                //         }
                // } else {
                //         if (e < -epsilon) {
                //                 slope_adjustment += adjustment_factor;
                //         } else if (e > epsilon) {
                //                 slope_adjustment -= adjustment_factor;
                //         }
                // }
        
                // set_output(left_signal * slope_adjustment, right_signal * slope_adjustment);
                // //set_output(left_signal, right_signal);
                
                // Serial.print(now);
                // Serial.print(" ");
                // Serial.print(left_speed, 5);
                // Serial.print(" ");
                // Serial.print(left_signal, 5);
                // Serial.print(" ");
                // Serial.print(instant_left_speed, 5);
                // Serial.print(" ");
                // Serial.print(slope_adjustment, 5);
                // Serial.println();

                // if (left_speed == 0.8) {
                //         set_target_speeds(-0.8, -0.8);
                // }
                // if (left_speed == -0.8) {
                //         set_target_speeds(0.8, 0.8);
                // }
                
                last_time = now;
        }
}

void loop()
{
        int test = get_test_id();
        
        switch (test){
        case 1:
                measure_voltage_and_current();
                break;

        case 2:
                system_identification(1);
                break;

        case 3:
                system_identification(-1);
                break;

        case 8:
                pi_controller();
                break;

        case 9:
                set_pulse_width(0, 0);
                exit(0);
                while (true) {}
                break;
                
        default:
                break;
        }
}

int get_test_id()
{
        Serial.println("Press enter the number of the test to run: ");
        Serial.println("1. Measure V(pw) and I(pw)");
        Serial.println("2. Measure speed for random signals - forward (system identification)");
        Serial.println("3. Measure speed for random signals - backward (system identification)");
        Serial.println("8. Test PI controller");
        Serial.println("9. Stop");

        int test = 0;
        while (test <= 0) {
                test = Serial.parseInt();
        }
        
        Serial.print("Running test ");
        Serial.println(test);

        return test;
}

//////////////////////////////////////////////////////////

void measure_voltage_and_current()
{
        unsigned long last_time = millis();
        double list[] = {
                0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
                0.6, 0.5, 0.4, 0.3, 0.2, 0.1,
                0.0, -0.1, -0.2, -0.3, -0.4, -0.5,
                -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0.0
        };
        int n = sizeof(list) / sizeof(double);

        Serial.println("Starting voltage and current measurements");
        
        for (int i = 0; i < n; i++) {

                double pw = list[i];
                Serial.print("pw=");
                Serial.println(pw);
                
                set_output(pw, pw);
                wait_for_enter();
                unsigned long now = millis();
                double dt = (now - last_time) / 1000.0;
                double instant_left_speed = left_encoder.get_speed(dt);
                double instant_right_speed = right_encoder.get_speed(dt);

                Serial.print(pw);
                Serial.print(" ");
                Serial.print(instant_left_speed, 5);
                Serial.print(" ");
                Serial.print(instant_right_speed, 5);
                Serial.println();
                
                last_time = now;
        }
        set_output(0.0, 0.0);
        Serial.println("Done");
}

void wait_for_enter()
{
        Serial.print("Press enter to continue: ");
        while (true) {
                if (Serial.available() > 0) {
                        int c = Serial.read();
                        if (c == '\n')
                                break;
                }
        }
        Serial.println("OK");
}

void system_identification(long direction)
{
        // normalized speed x µs x ticks/µs
        long pwmin = (long) (0.1 * 500.0 * 2.0);
        long pwmax = (long) (0.4 * 500.0 * 2.0);
        long delta = 0;
        long pw = 0;
        int measurements = 0;

        Serial.println("Collecting data for system identification");
        
        unsigned long last_time = millis();
        unsigned start_time = millis();
        int32_t last_left_position = left_encoder.get_position();
        int32_t last_right_position = right_encoder.get_position();
        
        while (true)  {
                unsigned long now = millis();
                unsigned long dt = now - last_time;
                if (now - last_time >= kUpdateInterval) {
                        int32_t left_position = left_encoder.get_position();
                        int32_t right_position = right_encoder.get_position();
        
                        Serial.print(millis() - start_time);
                        Serial.print(" ");
                        Serial.print(dt);
                        Serial.print(" ");
                        Serial.print(delta);
                        Serial.print(" ");
                        Serial.print(left_position - last_left_position);
                        Serial.print(" ");
                        Serial.print(right_position - last_right_position);
                        Serial.println();
        
                        counter++;
                        if (counter == N) {
                                counter = 0;
                                if (++measurements == 20)
                                        break;
                                delta = direction * random(pwmin, pwmax);
                                pw = 3000 + delta;
                                set_pulse_width(pw, pw);
                        }

                        last_time = now;
                        last_left_position = left_position;
                        last_right_position = right_position;
                }
        }
        
        set_pulse_width(3000, 3000);
        Serial.println("Done");
}


void pi_controller()
{
        Serial.println("PI controller");
        
        int measurements = 0;
        unsigned long last_time = millis();
        unsigned start_time = millis();
        int32_t last_left_position = left_encoder.get_position();
        int32_t last_right_position = right_encoder.get_position();

        long delta_target_left = 0;
        long delta_target_right = 0;
        double sum_left = 0.0;
        double sum_right = 0.0;
        int32_t sumi_left = 0;
        int32_t sumi_right = 0;
        
        while (true)  {
                unsigned long now = millis();
                long dt = now - last_time;
                if (now - last_time >= kUpdateInterval) {

                        int32_t left_position = left_encoder.get_position();
                        int32_t right_position = right_encoder.get_position();
                        int32_t delta_left = left_position - last_left_position;
                        int32_t delta_right = right_position - last_right_position;
                                                        
                        update_speeds(dt / 1000.0);
                        
                        // delta_target = speed rev/s * encoder_steps steps/rev * dt s;
                        // delta_target = speed * 2100 * (20 / 1000);
                        // delta_target = vi * 42000 / 1000;
                        // delta_target = speed * 42;
                        delta_target_left = (int32_t) (kDeltaCoeff * left_speed);
                        delta_target_right = (int32_t) (kDeltaCoeff * right_speed);

                        int32_t error_left = delta_target_left - delta_left;
                        int32_t error_right = delta_target_right - delta_right;
                        sumi_left += error_left;
                        sumi_right += error_right;

                        int32_t sp_left = 12 * error_left / 10;
                        int32_t sp_right = 12 * error_right / 10;
                        int32_t si_left = 9 * sumi_left / 100;
                        int32_t si_right = 9 * sumi_right / 100;
                        int32_t s_left = sp_left + si_left;
                        int32_t s_right = sp_right + si_right;
                        set_pulse_width(3000 + s_left, 3000 + s_right);

                        sum_left += error_left;
                        sum_right += error_right;
                        double proportional_left = (double) error_left *   0.0012;
                        double proportional_right = (double) error_right * 0.0012;
                        double integral_left = sum_left *   0.00009;
                        double integral_right = sum_right * 0.00009;
                        double output_left = proportional_left + integral_left;
                        double output_right = proportional_right + integral_right;

                        //set_output(output_left, output_right);
                        //set_output(left_speed, right_speed);
                        
                        Serial.print(millis() - start_time);
                        Serial.print(" ");
                        Serial.print(dt);
                        Serial.print(" ");
                        Serial.print(left_target, 5);
                        Serial.print(" ");
                        Serial.print(left_speed, 5);
                        Serial.print(" ");
                        Serial.print(right_speed, 5);
                        Serial.print(" ");
                        Serial.print(delta_target_left);
                        Serial.print(" ");
                        Serial.print(delta_target_right);
                        Serial.print(" ");
                        Serial.print(delta_left);
                        Serial.print(" ");
                        Serial.print(delta_right);
                        Serial.print(" ");
                        Serial.print(proportional_left, 5);
                        Serial.print(" ");
                        Serial.print(proportional_right, 5);
                        Serial.print(" ");
                        Serial.print(integral_left, 5);
                        Serial.print(" ");
                        Serial.print(integral_right, 5);
                        Serial.print(" ");
                        Serial.print(output_left, 5);
                        Serial.print(" ");
                        Serial.print(output_right, 5);
                        Serial.print(" ");
                        Serial.print(sp_left);
                        Serial.print(" ");
                        Serial.print(sp_right);
                        Serial.print(" ");
                        Serial.print(si_left);
                        Serial.print(" ");
                        Serial.print(si_right);
                        Serial.print(" ");
                        Serial.print(s_left);
                        Serial.print(" ");
                        Serial.print(s_right);
                        Serial.println();
        
                        counter++;
                        if (counter == M) {
                                counter = 0;
                                if (++measurements == 16)
                                        break;
                                int32_t vi = random(50, 400);
                                double speed = -vi / 1000.0;
                                set_target_speeds(speed, speed);
                        }
                        
                        last_time = now;
                        last_left_position = left_position;
                        last_right_position = right_position;
                }
        }
        
        set_pulse_width(3000, 3000);
        Serial.println("Done");
}


//////////////////////////////////////////////////////////

// static double signals[] = { -1.0, -0.95, -0.9, -0.85, -0.8, -0.75, -0.7, -0.65,
//                             -0.6, -0.55, -0.5, -0.45, -0.4, -0.35, -0.3, -0.25,
//                             -0.2, -0.15, -0.13, -0.12, -0.11, -0.10, -0.09, -0.08, -0.07, -0.06,
//                             0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.15, 0.2,
//                             0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6,
//                             0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0 };

static double signals[] = {
        0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55,
        0.5, 0.45, 0.4, 0.35, 0.3, 0.25, 0.2, 0.15, 0.13, 0.12, 0.11, 0.10, 0.09, 0.08, 0.0,
        -0.08, -0.09, -0.10, -0.11, -0.12, -0.13, -0.15, -0.2, -0.25, -0.3, -0.35, -0.4, -0.45, -0.5, -0.55,
        -0.5, -0.45, -0.4, -0.35, -0.3, -0.25, -0.2, -0.15, -0.13, -0.12, -0.11, -0.10, -0.09, -0.08, -0.0,
};

static int num_signals = sizeof(signals) / sizeof(double);
static int current_signal = -1;

void measure_signal_to_speed_curve(double dt)
{
        if (current_signal < 0) {
                current_signal = 0;
                left_signal = signals[current_signal];
                right_signal = signals[current_signal];
                set_output(left_signal, right_signal);
                return;
        }
        
        double instant_left_speed = left_encoder.get_speed(dt);
        double instant_right_speed = right_encoder.get_speed(dt);

        if (recording_index < N) {
                left_recordings[recording_index++] = instant_left_speed;
                right_recordings[recording_index++] = instant_right_speed;
        }
        counter++;
        if (counter == N) {

                // Skip the first measurements because the
                // motor is still transitioning between two
                // speeds;
                int offset = 1 + N/10;
                int num = recording_index - offset;
                double left_avg_speed = 0.0;
                double right_avg_speed = 0.0;
                
                for (int i = offset; i < num; i++) {
                        left_avg_speed += left_recordings[i];
                        right_avg_speed += right_recordings[i];
                }
                left_avg_speed /= num;
                right_avg_speed /= num;
                        
                Serial.print(left_signal, 5);
                Serial.print(" ");
                Serial.print(left_avg_speed, 5);
                Serial.print(" ");
                Serial.print(right_avg_speed, 5);
                Serial.println();
                
                recording_index = 0;
                counter = 0;
                current_signal++;
                
                if (current_signal >= 0
                    && current_signal < num_signals) {
                        left_signal = signals[current_signal];
                        right_signal = signals[current_signal];
                        set_output(left_signal, right_signal);
                } else {
                        left_signal = 0.0;
                        right_signal = 0.0;
                }
                
                set_output(left_signal, right_signal);
        }
}

//////////////////////////////////////////////////////////

struct SignalSpeedPoint {
        double signal;
        double speed;
};

static SignalSpeedPoint speed_to_signal[] = {
        { -1.00000, -1.02194 },
        { -0.95000, -0.97566 },
        { -0.90000, -0.89326 },
        { -0.85000, -0.81380 },
        { -0.80000, -0.73596 },
        { -0.75000, -0.66158 },
        { -0.70000, -0.58989 },
        { -0.65000, -0.52220 },
        { -0.60000, -0.45960 },
        { -0.55000, -0.40583 },
        { -0.50000, -0.34778 },
        { -0.45000, -0.29294 },
        { -0.40000, -0.24906 },
        { -0.35000, -0.20037 },
        { -0.30000, -0.15971 },
        { -0.25000, -0.12119 },
        { -0.20000, -0.08802 },
        { -0.15000, -0.05645 },
        { -0.13000, -0.04628 },
        { -0.12000, -0.04040 },
        { -0.11000, -0.03478 },
        { -0.10000, -0.03050 },
        { -0.09000, -0.02541 },
        { -0.08000, -0.01819 },
        { -0.07000, 0.00000 },
        { -0.06000, 0.00000 },
        { 0.06000, 0.00000 },
        { 0.07000, 0.00000 },
        { 0.08000, 0.00000 },
        { 0.09000, 0.01980 },
        { 0.10000, 0.02702 },
        { 0.11000, 0.03237 },
        { 0.12000, 0.03745 },
        { 0.13000, 0.04307 },
        { 0.15000, 0.05457 },
        { 0.20000, 0.08454 },
        { 0.25000, 0.11878 },
        { 0.30000, 0.15730 },
        { 0.35000, 0.19904 },
        { 0.40000, 0.24478 },
        { 0.45000, 0.29454 },
        { 0.50000, 0.34644 },
        { 0.55000, 0.40396 },
        { 0.60000, 0.46897 },
        { 0.65000, 0.53077 },
        { 0.70000, 0.59684 },
        { 0.75000, 0.66827 },
        { 0.80000, 0.74157 },
        { 0.85000, 0.81782 },
        { 0.90000, 0.91145 },
        { 0.95000, 0.99197 },
        { 1.00000, 1.03451 }
};

static uint8_t num_speed_to_signal_points = sizeof(speed_to_signal) / sizeof(SignalSpeedPoint);

uint8_t find_signal_index(double speed)
{
        for (uint8_t i = 0; i < num_speed_to_signal_points - 1; i++) {
                if (speed_to_signal[i].speed <= speed
                    && speed < speed_to_signal[i+1].speed) {
                        return i;
                }
        }
}

double interpolate_signal(double v, uint8_t i)
{
        double v0 = speed_to_signal[i].speed;
        double v1 = speed_to_signal[i+1].speed;
        double s0 = speed_to_signal[i].signal;
        double s1 = speed_to_signal[i+1].signal;
        double delta = (s1 - s0) * (v - v0) / (v1 - v0);
        double s = s0 + delta;
        return s;
}

double estimate_signal(double normalized_speed, double max_speed)
{
        double absolute_speed = normalized_speed * max_speed;
        double signal = 0.0;
        if (normalized_speed != 0.0) {
                uint8_t index = find_signal_index(absolute_speed);
                signal = interpolate_signal(absolute_speed, index);
        }
        return signal;
}

static double speeds[] = { -0.1, -0.2, -0.3, -0.4, -0.5, -0.6, -0.7, -0.8, -0.9,
                           -1.0, -0.95, -0.9, -0.85, -0.8, -0.75, -0.7, -0.65,
                           -0.6, -0.55, -0.5, -0.45, -0.4, -0.35, -0.3, -0.25,
                           -0.2, -0.15, -0.1, -0.05, 0.0, 
                           0.05, 0.10, 0.15, 0.2,
                           0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6,
                           0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0,
                           0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };
static int num_speeds = sizeof(speeds) / sizeof(double);
static int current_speed = -1;

void test_speed_to_signal_curve(double max_speed, double dt)
{
        if (current_speed == -1) {
                current_speed = 0;
                double v = speeds[current_speed];
                double s = estimate_signal(v, max_speed);
                left_signal = s;
                right_signal = s;
                set_output(left_signal, right_signal);
                return;
        }
        
        double instant_left_speed = left_encoder.get_speed(dt);
        double instant_right_speed = right_encoder.get_speed(dt);

        Serial.print(millis());
        Serial.print(" ");
        Serial.print(speeds[current_speed], 5);
        Serial.print(" ");
        Serial.print(instant_left_speed / max_speed, 5);
        Serial.print(" ");
        Serial.print(speeds[current_speed] * max_speed, 5);
        Serial.print(" ");
        Serial.print(instant_left_speed, 5);
        Serial.print(" ");
        Serial.print(left_signal, 5);
        Serial.println();
        
        if (recording_index < N)
                left_recordings[recording_index++] = instant_left_speed;

        counter++;
        if (counter == N) {

                // Skip the first measurements because the
                // motor is still transitioning between two
                // speeds;
                int offset = 1 + N/10;
                int num = recording_index - offset;
                double left_avg_speed = 0.0;
                
                for (int i = offset; i < num; i++) {
                        left_avg_speed += left_recordings[i];
                }
                left_avg_speed /= num;
                
                recording_index = 0;
                counter = 0;
                current_speed++;
                
                if (current_speed >= 0
                    && current_speed < num_speeds) {
                        double v = speeds[current_speed];
                        double s = estimate_signal(v, max_speed);
                                
                        left_signal = s;
                        right_signal = s;
                                
                        set_output(left_signal, right_signal);
                } else {
                        left_signal = 0.0;
                        right_signal = 0.0;
                }
                        
                set_output(left_signal, right_signal);
        }
}

//////////////////////////////////////////////////////////

void test_speed_to_signal_curve_with_pid(double max_speed, double dt)
{
        static double sum = 0.0;
        static double e_prev = 0.0;
        
        if (current_speed == -1) {
                current_speed = 0;
                double v = speeds[current_speed];
                double s = estimate_signal(v, max_speed);
                left_signal = s;
                right_signal = s;
                set_output(left_signal, right_signal);
                return;
        }
        
        double instant_left_speed = left_encoder.get_speed(dt);
        double instant_right_speed = right_encoder.get_speed(dt);

        //double left_avg_speed = instant_left_speed;
        double left_avg_speed = left_filter.compute(instant_left_speed);
        
        double kp = 0.0;
        double ki = 0.0;
        double kd = 0.0;

        double vt = speeds[current_speed];
        double vm = left_avg_speed / max_speed;
        double e = vt - vm;
        double sp = kp * e;
                
        sum += e * dt;
        double si = ki * sum;
                
        double de = (e - e_prev) / dt;
        double sd = kd * de;
                
        double ds  = sp + si + sd;
                        
        set_output(left_signal + ds, right_signal + ds);

        Serial.print(millis());
        Serial.print(" ");
        Serial.print(speeds[current_speed], 5);
        Serial.print(" ");
        Serial.print(left_avg_speed / max_speed, 5);
        Serial.print(" ");
        Serial.print(speeds[current_speed] * max_speed, 5);
        Serial.print(" ");
        Serial.print(left_avg_speed, 5);
        Serial.print(" ");
        Serial.print(left_signal, 5);
        Serial.print(" ");
        Serial.print(e, 5);
        Serial.print(" ");
        Serial.print(sp, 5);
        Serial.print(" ");
        Serial.print(si, 5);
        Serial.print(" ");
        Serial.print(sd, 5);
        Serial.print(" ");
        Serial.print(ds, 5);
        Serial.print(" ");
        Serial.print(left_signal+ds, 5);
        Serial.println();
        
        counter++;
        if (counter == N) {
                counter = 0;
                current_speed++;
                
                if (current_speed >= 0
                    && current_speed < num_speeds) {
                        double v = speeds[current_speed];
                        double s = estimate_signal(v, max_speed);
                                
                        left_signal = s;
                        right_signal = s;
                                
                        set_output(left_signal, right_signal);
                } else {
                        left_signal = 0.0;
                        right_signal = 0.0;
                }
                        
                set_output(left_signal, right_signal);
        }
}

//////////////////////////////////////////////////////////

double speed_adjustment = 0.0;

void test_speed_to_signal_curve_with_stepup_stepdown(double max_speed, double dt)
{
        static double sum = 0.0;
        static double e_prev = 0.0;
        
        if (current_speed == -1) {
                current_speed = 0;
                double v = speeds[current_speed];
                double s = estimate_signal(v, max_speed);
                left_signal = s;
                right_signal = s;
                set_output(left_signal, right_signal);
                return;
        }
        
        double instant_left_speed = left_encoder.get_speed(dt);
        double instant_right_speed = right_encoder.get_speed(dt);

        double left_avg_speed = left_filter.compute(instant_left_speed);

        double vt = speeds[current_speed];
        double vm = left_avg_speed / max_speed;
        double e = vt - vm;
    
        /*
          speed 0.2   -> signal 0.08454 -> pw 42  (=0.08454 * 500)
          speed 0.25  -> signal 0.11878 -> pw 59

          d(speed) = 0.05   -> d(pw) = 17
          d(pw) = 1         -> d(speed) = 0.0029412

          d(speed)/v_max = 0.0029412 / 0.6 = 0.0049
          epsilon = dv/v_max/2 = 0.00245

         */
        double epsilon = 0.00245;
        if (e > epsilon) {
                speed_adjustment += 1.0 / 500.0;
        } else if (e < -epsilon) {
                speed_adjustment -= 1.0 / 500.0;
        }
        
        set_output(left_signal + speed_adjustment, right_signal + speed_adjustment);
        //set_output(left_signal, right_signal);

        Serial.print(millis());
        Serial.print(" ");
        Serial.print(speeds[current_speed], 5);
        Serial.print(" ");
        Serial.print(left_avg_speed / max_speed, 5);
        Serial.print(" ");
        Serial.print(speeds[current_speed] * max_speed, 5);
        Serial.print(" ");
        Serial.print(left_avg_speed, 5);
        Serial.print(" ");
        Serial.print(left_signal, 5);
        Serial.print(" ");
        Serial.print(e, 5);
        Serial.print(" ");
        Serial.print(speed_adjustment, 5);
        Serial.print(" ");
        Serial.print(left_signal + speed_adjustment, 5);
        Serial.println();
        
        counter++;
        if (counter == N) {
                counter = 0;
                current_speed++;
                
                if (current_speed >= 0
                    && current_speed < num_speeds) {
                        double v = speeds[current_speed];
                        double s = estimate_signal(v, max_speed);
                        speed_adjustment = 0.0;
                        
                        left_signal = s;
                        right_signal = s;
                                
                        set_output(left_signal, right_signal);
                } else {
                        left_signal = 0.0;
                        right_signal = 0.0;
                        speed_adjustment = 0.0;
                }
                        
                set_output(left_signal, right_signal);
        }
}

//////////////////////////////////////////////////////////

void test_speed_to_signal_curve_with_slope_adjustment(double max_speed, double dt)
{
        static double sum = 0.0;
        static double e_prev = 0.0;
        
        if (current_speed == -1) {
                current_speed = 0;
                double v = speeds[current_speed];
                double s = estimate_signal(v, max_speed);
                left_signal = s;
                right_signal = s;
                set_output(left_signal, right_signal);
                return;
        }
        
        double instant_left_speed = left_encoder.get_speed(dt);
        double instant_right_speed = right_encoder.get_speed(dt);

        double left_avg_speed = left_filter.compute(instant_left_speed);

        double vt = speeds[current_speed];
        double vm = left_avg_speed / max_speed;
        double e = vt - vm;

        double epsilon = 0.00245;
        if (vt > 0) {
                if (e > epsilon) {
                        slope_adjustment += adjustment_factor;
                } else if (e < -epsilon) {
                        slope_adjustment -= adjustment_factor;
                }
        } else {
                if (e < -epsilon) {
                        slope_adjustment += adjustment_factor;
                } else if (e > epsilon) {
                        slope_adjustment -= adjustment_factor;
                }
        }
        
        set_output(left_signal * slope_adjustment, right_signal * slope_adjustment);
        //set_output(left_signal, right_signal);

        Serial.print(millis());
        Serial.print(" ");
        Serial.print(speeds[current_speed], 5);
        Serial.print(" ");
        Serial.print(left_avg_speed / max_speed, 5);
        Serial.print(" ");
        Serial.print(speeds[current_speed] * max_speed, 5);
        Serial.print(" ");
        Serial.print(left_avg_speed, 5);
        Serial.print(" ");
        Serial.print(left_signal, 5);
        Serial.print(" ");
        Serial.print(e, 5);
        Serial.print(" ");
        Serial.print(slope_adjustment, 5);
        Serial.print(" ");
        Serial.print(left_signal + speed_adjustment, 5);
        Serial.println();
        
        counter++;
        if (counter == N) {
                counter = 0;
                current_speed++;
                
                if (current_speed >= 0
                    && current_speed < num_speeds) {
                        double v = speeds[current_speed];
                        double s = estimate_signal(v, max_speed);
                        speed_adjustment = 0.0;
                        
                        left_signal = s;
                        right_signal = s;
                                
                        set_output(left_signal, right_signal);
                } else {
                        left_signal = 0.0;
                        right_signal = 0.0;
                        speed_adjustment = 0.0;
                }
                        
                set_output(left_signal, right_signal);
        }
}

