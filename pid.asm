.text
# initialize
    LI R4, 0        # integral_accumulator = 0
    LI R5, 0        # previous_error = 0


LOOP:
    # read encoder data
    LI R7 0xFF00        # encoder MMIO address
    LD R1, R7           # R1 = encoder value

    # read setpoint
    LI R7, 0xFF04
    LD R2, R7           # R2 = setpoint

    # calculate error
    SUB R3, R2, R1      # R3 = error = setpoint - encoder

    # calculate proportional term
    # Kp = 1 -> P = error in R3
    # TODO: support other values of Kp

    # calculate integral term
    ADD R4, R4, R3      # integral_accumulator += error

    # calculate derivative term
    SUB R7, R3, R5      # R7 = derivative = error - previous_error

    # compute PID output
    ADD R6, R3, R4      # P and I terms
    ADD R6, R6, R7      # P, I, and D terms

    # output to motor
    LI R7, 0xFF02
    ST R6, R7           # write output to motor controller

    # update previous_error
    MV R5, R3

    # repeat control loop
    J LOOP

.data
    encoder_MMIO: .word 0xFF00
    motor_controller_MMIO: .word 0xFF02
    setpoint_MMIO: .word 0xFF04

