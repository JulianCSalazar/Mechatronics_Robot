#define FrontLeft 0x02
#define FrontRight 0x01
#define middle 0x08
#define BackLeft 0x04
#define BackRight 0x10
#define Front 0x03
#define Back 0x14
#define FrontLBackR 0x12
#define FrontRBackL 0x05
#define FrontMid 0x0B
#define Obstacle 0x00

void Tape_Init(void);

unsigned int Tape_Read(void);