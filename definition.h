//
//  definition.h
//  Hungry_snake
//
//  Created by 周沫 on 2020-04-05.
//  Copyright © 2020 周沫. All rights reserved.
//

#define USER_MODE 0b10000
#define FIQ_MODE 0b10001
#define IRQ_MODE 0b10010
#define SVC_MODE 0b10011
#define ABORT_MODE 0b10111
#define UNDEF_MODE 0b11011
#define SYS_MODE 0b111111

#define INT_ENABLE 0b01000000
#define INT_DISABLE 0b11000000
#define ENABLE 0b1

#define emptyPixel 0
