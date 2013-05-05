/*
 Button.pde - GUI Button for GameOfLight graphics editor
 Copyright (c) 2013 Sigmund Hansen.  All right reserved.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * Simple class for GUI buttons.
 */
abstract class Button {
  /** Position. */
  int x, y;
  /** Size. */
  int width, height;
  /** Label. */
  String t;
  
  /**
   * Instantiates a button.
   *
   * @param x The horizontal position.
   * @param y The vertical position.
   * @param width The width of the button.
   * @param height The height of the button.
   * @param t The label shown on the button.
   */
  Button(int x, int y, int width, int height, String t) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
    this.t = t;
    
    if (width < 0) {
      this.x += width;
      this.width = -width;
    }
  }
  
  /**
   * Draws a button on the screen at its position,
   * handling mouse overs by darkening the button.
   */
  void draw() {
    stroke(0);
    
    if (isInBounds(mouseX, mouseY)) {
      fill(128);
    } else {
      fill(255);
    }
    
    rect(x, y, width, height);
    fill(0);
    text(t, x + width / 2, y + (height * 3) / 4);
  }
  
  /**
   * Test whether the given coordinates are inside the button.
   *
   * @param x The X coordinate to test.
   * @param y The Y coordinate to test.
   * @return true if the coordinates are inside the button bounds,
   *         false if they are outside.
   */
  boolean isInBounds(int x, int y) {
    return x > this.x && y > this.y && x < this.x + width && y < this.y + height;
  }
  
  abstract void press(int x, int y);
}

