// SBUS defines
#define SBUSCENTER 992
#define SBUSDEADBAND 10
#define SBUSMIN 172
#define SBUSMAX 1811
#define MAXSBUSOK 200

#define marcDuinoBaudRate 9600 //baud rate for MarcDuino controllers
#define turnspeed 50    //turnspeed is used by the BHD mixing algorythm. might look into making this more dynamic in the future.
#define serialFootLatency 25  //This is a delay factor in ms to prevent queueing of the Serial data.
#define serialDomeLatency 25  //This is a delay factor in ms to prevent queueing of the Serial data.
// static constexpr int8_t NUM_CH(16)

// Servo States
#define ServoPassThrough 0
#define ServoSequences 1






