  /*
  About - credits for the GameOfLight libraries
  Copyright (c) 2013 Stian Selbek.  All right reserved.

  This file is part of Game Of Light.

  Game Of Light is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Game Of Light is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Game Of Light.  If not, see <http://www.gnu.org/licenses/>.
  */

#include <GameOfLightHW.h>
#include <avr/pgmspace.h>

extern GameOfLightHW frame;
uint8_t about_colour = 3;

uint8_t about_pacghost1[] PROGMEM = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xe0, 0x0, 0xf8, 0x0, 0xf8, 0x80, 0xf8,
0x80, 0xf8, 0x80, 0xfe, 0x80, 0xfe, 0x80, 0xfe, 0x80, 0xfe, 0x0, 0xfe, 0x0, 0xfe, 0x80, 0xf8,
0x80, 0xf8, 0x80, 0xf8, 0x80, 0xf8, 0x80, 0xe0, 0x80, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0xfe, 0x0, 0xfe, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x1f, 0xff,
0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x19, 0xf9, 0x19, 0xf9, 0x0, 0xff, 0x0, 0xff, 0x1f, 0xff,
0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x19, 0xf9, 0x19, 0xf9, 0x0, 0xfe, 0x0, 0xfe, 0x0, 0x0,
0x0, 0x0, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x7f,
0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f,
0x0, 0x7f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x0
};

uint8_t about_pacghost2[] PROGMEM = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x0, 0xe0, 0x0, 0xf8, 0x0, 0xf8, 0x80, 0xf8,
0x80, 0xf8, 0x80, 0xfe, 0x80, 0xfe, 0x80, 0xfe, 0x80, 0xfe, 0x0, 0xfe, 0x0, 0xfe, 0x80, 0xf8,
0x80, 0xf8, 0x80, 0xf8, 0x80, 0xf8, 0x80, 0xe0, 0x80, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0xfe, 0x0, 0xfe, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x0, 0xff, 0x1f, 0xff,
0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x19, 0xf9, 0x19, 0xf9, 0x0, 0xff, 0x0, 0xff, 0x1f, 0xff,
0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x19, 0xf9, 0x19, 0xf9, 0x0, 0xfe, 0x0, 0xfe, 0x0, 0x0,
0x0, 0x0, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f,
0x0, 0x1f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x1f,
0x0, 0x1f, 0x0, 0x1f, 0x0, 0x1f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x7f, 0x0, 0x0
};


PROGMEM char about_p0[] = "Abdimaalik Abirahmaan";
PROGMEM char about_p1[] = "Martin Holos";
PROGMEM char about_p2[] = "Andre Kramer Orten";
PROGMEM char about_p3[] = "Eivind Wikheim";
PROGMEM char about_p4[] = "and";
PROGMEM char about_p5[] = "Stian Selbek";

PROGMEM char* about_credits[] = {about_p0, about_p1, about_p2, about_p3, about_p4, about_p5};

void about_run() {
	int8_t i, line;

	frame.clear();
	about_title();
	frame.update();
	delay(2000);
	for (i = 0; i < 63; i++) {
		frame.scrollLeft();
		frame.update();
		delay(5);
	}

	frame.clear();
	frame.gotoXY(14, 3);
	frame.print("A 1510", RED);
	frame.gotoXY(14, 4);
	frame.print("Design", ORANGE);
	frame.gotoXY(14, 5);
	frame.print("Project", GREEN);

	frame.update();
	delay(2000);
	for (i = 0; i < 63; i++) {
		frame.scrollRight();
		frame.update();
		delay(5);
	}

	frame.clear();
	frame.gotoXY(24, 4);
	frame.print("by", ORANGE);
	frame.update();
	delay(1500);

	for (i = 0; i < 6; i++) {
		frame.clear();
		about_printText(about_credits, i);
		frame.gotoXY(0, 0);
		frame.clear(63); //clear scratch area
		frame.update();
		delay(1500);
	}
	frame.clear();

	about_title();
	frame.update();
	delay(1000);

	for (i = -23; i < 87; i++) {
		//delay/clearing of ghost path
		frame.gotoXY(0, 5);
		frame.clear(64);
		frame.gotoXY(0, 6);
		frame.clear(64);
		frame.gotoXY(0, 7);
		frame.clear(64);

		//Repaint character
		for (uint8_t y = 0; y < 3; y++) {
   			for (uint8_t x = 0; x < 3; x++) {
      			frame.blit(((i & 4) ? about_pacghost1 : about_pacghost2) + x*16+y*48, i+x*8, 40+y*8);
    		}
  		}
  		delay(30);
  		frame.update();
	}

	/*delay(500);
	//heart curve around title?
	int8_t x, y;
	for (float t = -3.14; t < 3.14; t+=0.01) {
   		x = 16*sin(t);
   		y = 13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t);
   		frame.setPixel(32+x*1.35,32-y*1.35, RED);
   		delay(10);
	}
	frame.update();*/

	delay(3000);

}

void about_splash() {
	frame.clear();
	frame.gotoXY(15,1);
	frame.print("About:", GREEN);
	about_title();
}


void about_title() {
	frame.gotoXY(14, 3);
	frame.print("Game", RED);
	frame.gotoXY(14+26, 3);
	frame.print("of", ORANGE);
	frame.gotoXY(17, 4);
	frame.print("Light", GREEN);	
}


void about_printText(PROGMEM char* data[], uint8_t i) {
	uint8_t words = 0, j, length[3], sum;
	char *ptr = (char*)&frame.buff[0][0];

	//Use upper line as buffer for the text
	strcpy_P(ptr, (char*)pgm_read_word(&data[i]));

	//Get word count
	j = 0; sum = 0;
	while(*(ptr + j) != '\0') {
		//Serial.println(*(ptr + j));
		if (*(ptr + j) == ' ') {
			length[words] = words == 0 ? j : (j - sum - words);
			sum += length[words];
			//Serial.println(length[words]);
			//Serial.println(sum);
			words++;
		}
		j++;
	}

	if (words != 0) {
		length[words] = j - sum - words;
	} else {
		length[words] = j;
	}

	for (i = 0; i <= words; i++) {
		frame.gotoXY(32 - length[i]*6/2, (3-(words/2)) + i);
		frame.setColour(about_colour--);
		if (about_colour == 0) about_colour = 3;
		for (j = 0; j < length[i]; j++) {
			frame.print(*ptr);
			ptr++;
		}
		ptr++; //spaces
	}

	/*j = 0;
	while(*(ptr + j) != '\0') {
		//Serial.println(*(ptr + j));
		if (*(ptr + j) == ' ') {
			words++;
		}
		j++;
	}*/

	//Serial.println(words);

	//pgm_read_byte(font+
}