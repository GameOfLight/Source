/*
  GOLEditor.pde - Graphics editor for GameOfLight
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
import java.awt.event.*;
import java.util.Scanner;

/*
 * GOLEditor is a simple graphics editor
 * that can be used to create texture arrays for GameOfLight.
 *
 * A single texture can be edited at a time, and the size is set to
 * 8x8 pixels/LEDs. This means a single tile covers one LED matrix
 * in the GameOfLight screen.
 *
 * The size was chosen to reduce the number of LED matrix arrays
 * the sprites could span, simplifying "blitting" on the machine.
 *
 * Tiles can be imported/exported to/from images and C arrays.
 * Supported image formats are Targa (tga), TIFF (tif), PNG and JPG.
 *
 * Note that if you export tiles to images, you may end up with one or
 * more empty tiles at the end. Currently the editor does not handle
 * deletion or skipping empty tiles in an imported tile set.
 *
 * The left mouse button draws a pixel in the current color. The right
 * mouse button clears a pixel. The mouse wheel or up and down arrow
 * keys switches colors. The left and right arrow keys switches
 * tiles. The + key adds a new tile.
 */

/** Size of a side in a tile (8 covers one LED matrix). */
final int size = 8;
/** All tiles are stored here. */
ArrayList<int[]> images = new ArrayList<int[]>();
/** The tile pixel buffer. */
int[] buf = new int[size * size];
/** Available colors, matching the LEDs on the machine. */
color[] colors = {
  #000000, 
  #00d020, 
  #ff0000, 
  #ffa000
};

/** Mouse coordinates converted to tile pixels. */
int mX, mY;
/** The currently selected color for drawing. */
int curColor = 1;
/** The currently selected tile/image. */
int curImage = 0;

/** Whether the mouse cursor is currently shown. */
boolean cursor = false;

Button[] buttons;

/**
 * Prepare the window, tile list and add a mouse wheel listener.
 */
void setup() {
  images.add(buf);
  size(size * 20 + size * 10, size * 20 + 64);
  background(0);
  addMouseWheelListener(new MouseWheelListener() {
    public void mouseWheelMoved(MouseWheelEvent e) {
      mouseWheeled(e.getWheelRotation());
    }
  }
  );
  noCursor();
  frameRate(30);
  textAlign(CENTER);

  buttons = new Button[] {
    // Import button
    new Button(10, size * 20 + 10, width / 4, 20, "Import") {
      /**
       * Open a file chooser for imports.
       */
      public void press(int x, int y) {
        if (isInBounds(x, y)) {
          selectInput("Select a file to import sprites from:", "importImages");
        }
      }
    }
    , 
    // Export button
    new Button(width - 10, size * 20 + 10, -width / 4, 20, "Export") {
      /**
       * Open a file chooser for exports.
       */
      public void press(int x, int y) {
        if (isInBounds(x, y)) {
          selectInput("Select a file to export the sheet to:", "exportImages");
        }
      }
    }
    , 
    // Copy button
    new Button(10, size * 20 + 40, width / 4, 20, "Add copy") {
      /**
       * Adds a new tile with the same data as this tile.
       */
      public void press(int x, int y) {
        if (!isInBounds(x, y)) {
          return;
        }

        int[] tmp = new int[size * size];
        arrayCopy(buf, tmp);
        curImage = images.size();
        images.add(tmp);
        buf = tmp;
        drawBuffer();
      }
    }
    , 
    // Button for flipping the tile horizontally
    new Button(width / 2 - width / 8, size * 20 + 40, width / 4, 20, "Flip hor") {
      /**
       * Mirrors the currently selected tile.
       */
      public void press(int x, int y) {
        if (!isInBounds(x, y)) {
          return;
        }

        for (int i = 0; i < size / 2; i++) {
          for (int j = 0; j < size; j++) {
            buf[i + j * size] ^= buf[(size - 1 - i) + j * size];
            buf[(size - 1 - i) + j * size] ^= buf[i + j * size];
            buf[i + j * size] ^= buf[(size - 1 - i) + j * size];
          }
        }

        drawBuffer();
      }
    }
    , 
    // Button for flipping the tile horizontally
    new Button(width - 10, size * 20 + 40, -width / 4, 20, "Flip ver") {
      /**
       * Mirrors the currently selected tile.
       */
      public void press(int x, int y) {
        if (!isInBounds(x, y)) {
          return;
        }

        for (int i = 0; i < size; i++) {
          for (int j = 0; j < size / 2; j++) {
            buf[i + j * size] ^= buf[i + (size - 1 - j) * size];
            buf[i + (size - 1 - j) * size] ^= buf[i + j * size];
            buf[i + j * size] ^= buf[i + (size - 1 - j) * size];
          }
        }

        drawBuffer();
      }
    }
  };
}

/**
 * Draws lines to separate differently sized copies of the tile.
 * Draws GUI buttons.
 */
void draw() {
  stroke(255);
  line(0, size * 20, width, size * 20);
  line(size * 20, 0, size * 20, height);
  line(size * 20, size * 10, width, size * 10);
  noStroke();
  fill(192);
  rect(0, height, width, -64);
  for (Button button : buttons) {
    button.draw();
  }
  text("" + (curImage + 1), width / 2, size * 20 + 25);
}

/**
 * Draw the whole tile to the screen.
 */
void drawBuffer() {
  noStroke();
  for (int x = 0; x < size; x++) {
    for (int y = 0; y < size; y++) {
      fill(colors[buf[x + y * size]]);
      rect(x * 20, y * 20, 20, 20);
      rect(size * 20 + x * 10, y * 10, 10, 10);
      rect(size * 20 + x * 5, size * 10 + y * 5, 5, 5);
    }
  }
}

/**
 * Color a pixel if the mouse is in the drawing area.
 *
 * Handle button presses.
 */
void mousePressed() {
  int i = mX + mY * size;

  noStroke();
  if (mouseY < size * 20) {
    if (mouseButton == LEFT) { 
      buf[i] = curColor;
    } 
    else {
      buf[i] = 0;
    }
    fill(colors[buf[i]]);
    rect(mX * 20, mY * 20, 20, 20);
    rect(size * 20 + mX * 10, mY * 10, 10, 10);
    rect(size * 20 + mX * 5, size * 10 + mY * 5, 5, 5);
    set(mouseX, mouseY, #ffffff);
  } 
  else {
    for (Button button : buttons) {
      button.press(mouseX, mouseY);
    }
  }
}

/**
 * Simple hack to handle mouse dragging.
 * Move the mouse and color the pixel.
 *
 * Mouse pressed is called first to avoid a graphical artifact.
 */
void mouseDragged() {
  mouseMoved();
  mousePressed();
}

/**
 * Update the pixel position of the mouse if the mouse is in the
 * drawing area. Turn the cursor on or off if necessary.  Draws a
 * white pixel where the mouse is located (helps locate the mouse in
 * the drawing area) and erases it by drawing a black pixel where the
 * mouse was previously located.
 */
void mouseMoved() {
  int x = mouseX / 20;
  int y = mouseY / 20;
  int i = mX + mY * size;
  noStroke();

  if (mouseY < size * 20 && !mousePressed) {
    set(pmouseX, pmouseY, 0);
  }
  fill(colors[buf[i]]);
  rect(mX * 20, mY * 20, 20, 20);
  rect(size * 20 + mX * 10, mY * 10, 10, 10);
  rect(size * 20 + mX * 5, size * 10 + mY * 5, 5, 5);

  if (y >= size) {
    if (!cursor) {
      cursor = true;
      cursor();
    }
    return;
  } 
  else if (cursor) {
    cursor = false;
    noCursor();
  }

  if (x >= size) {
    x = mouseX / 10;
    y = mouseY / 10;
    x -= size * 2;
  }

  if (y >= size) {
    x = mouseX / 5;
    y = mouseY / 5;
    x %= size;
    y %= size;
  }

  mX = x;
  mY = y;

  fill(colors[curColor] & #AAAAAA);
  rect(mX * 20, mY * 20, 20, 20);
  rect(size * 20 + mX * 10, mY * 10, 10, 10);
  rect(size * 20 + mX * 5, size * 10 + mY * 5, 5, 5);
  set(mouseX, mouseY, #ffffff);
}

/**
 * Handle mouse wheel events, changing colors.
 */
void mouseWheeled(int mouseZ) {
  curColor += mouseZ + 2;
  curColor %= 3;
  curColor++;
  noStroke();
  mouseMoved();
}

/**
 * Switch tiles, color or add tiles.
 */
void keyReleased() {
  if (keyCode == LEFT) {
    switchImage(-1);
  }
  else if (keyCode == RIGHT) {
    switchImage(1);
  }
  else if (keyCode == UP) {
    mouseWheeled(-1);
  }
  else if (keyCode == DOWN) {
    mouseWheeled(1);
  } 
  else if (key == '+') {
    addImage();
  }
}

/**
 * Move through the tiles.
 */
void switchImage(int dir) {
  curImage += dir + images.size();
  curImage %= images.size();
  buf = images.get(curImage);
  drawBuffer();
}

/**
 * Add an image to the tile set and switch to it.
 */
void addImage() {
  buf = new int[size * size];
  curImage = images.size();
  images.add(buf);
  drawBuffer();
}

/**
 * Handle file chooser results by calling the appropriate export method.
 */
void exportImages(File f) {
  if (f == null) {
    return;
  }

  if (f.getName().endsWith(".png") || f.getName().endsWith(".jpg") || f.getName().endsWith(".tga") || f.getName().endsWith(".tif")) {
    exportToImage(f);
  } 
  else {
    exportToCArray(f);
  }
}

/**
 * Exports the image to an image file.
 */
void exportToImage(File f) {
  int square = ceil(sqrt(images.size()));
  PImage img = createImage(square * size, square * size, RGB);

  int current = 0;
  for (int[] buf : images) {
    int startX = (current * size) % (img.width);
    int startY = ((current * size) / (img.width)) * size;

    for (int i = 0; i < buf.length; i++) {
      img.set(startX + (i % size), startY + (i / size), colors[buf[i]]);
    }
    current++;
  }

  while (current < square * square) {
    int startX = (current * size) % (img.width);
    int startY = ((current * size) / (img.width)) * size;

    for (int i = 0; i < buf.length; i++) {
      img.set(startX + (i % size), startY + (i / size), 0);
    }
    current++;
  }

  img.save(f.getAbsolutePath());
}

/**
 * Export images to a C array suited for use on the GameOfLight machine.
 */
void exportToCArray(File f) {
  int[] buf;
  PrintWriter pw = null;
  try {
    String sheetName = f.getName();
    int lastDot = sheetName.lastIndexOf('.');
    sheetName = lastDot > 0 ? sheetName.substring(0, lastDot) : sheetName;
    sheetName = sheetName.replaceAll("[\\s\\.*-+/\\\\]", "_");

    pw = new PrintWriter(f);
    pw.println("uint8_t " + sheetName + "[] PROGMEM = {");
    for (int k = 0; k < images.size(); k++) {
      buf = images.get(k);
      for (int i = 0; i < size; i++) {
        byte g = 0;
        byte r = 0;
        for (int j = 0; j < size; j++) {
          g |= (buf[i + size * j] & 1) << j;
          r |= ((buf[i + size * j] >> 1) & 1) << j;
        }
        pw.printf("0x%x, 0x%x%s", g, r, i == size - 1 ? "" : ", ");
      }
      pw.println(k == images.size() - 1 ? "" : ",");
    }
    pw.println("};");
    pw.close();
  } 
  catch (IOException ioe) {
    System.err.printf("Could not write to file:%n%s%n", ioe.getMessage());

    if (pw != null) {
      pw.close();
    }
  }
}

/**
 * Handle file chooser results by calling the appropriate import method.
 */
void importImages(File f) {
  if (f == null) {
    return;
  }

  boolean clear = false;
  if (images.size() == 1) {
    clear = true;
    for (int i = 0; i < buf.length; i++) {
      if (buf[i] != 0) {
        clear = false;
        break;
      }
    }
  }

  if (clear) {
    images.clear();
  }

  try {
    if (f.getName().endsWith(".png") || f.getName().endsWith(".jpg") || f.getName().endsWith(".tga") || f.getName().endsWith(".tif")) {
      importImagesFromPImage(loadImage(f.getAbsolutePath()));
    } 
    else {
      importImagesFromCArray(new Scanner(f));
    }
  } 
  catch (IOException ioe) {
    System.err.printf("Could not read file:%n%s%n", ioe.getMessage());
  }
  switchImage(0);
}

/**
 * Imports images from a PImage (usually loaded from a raster graphics file).
 */
void importImagesFromPImage(PImage p) {
  if (p.width % size != 0 || p.height % size != 0) {
    System.err.printf("Only images with dimensions divisible by %d is supported.%n", size);
  }

  int xSprites = p.width / size;
  int ySprites = p.height / size;
  int curPixel = 0;

  while (curPixel < p.width * p.height) {
    buf = new int[size * size];
    images.add(buf);
    for (int i = 0; i < size * size; i++, curPixel++) {
      int curSquare = curPixel / (size * size);
      int ix = (curSquare % xSprites) * size + curPixel % size;
      int iy = (curSquare / xSprites) * size + ((curPixel / size) % size);
      int c = p.get(ix, iy);
      if (alpha(c) < 128 || brightness(c) < 64) {
        c = 0;
      } 
      else if (saturation(c) < 64) {
        c = 3;
      } 
      else if (hue(c) > 48 && hue(c) <= 128) {
        c = 1;
      } 
      else if (hue(c) < 24 || hue(c) > 128) {
        c = 2;
      } 
      else {
        c = 3;
      }
      buf[i] = c;
    }
  }
}

/**
 * Imports tiles from a C array (the file should not contain other code).
 */
void importImagesFromCArray(Scanner s) throws IOException {
  s.useDelimiter("[,;x\\s}{]+");
  if (s.hasNextLine()) {
    s.nextLine();
  }

  int i = 0;
  while (s.hasNextInt ()) {
    if (i == 0) {
      buf = new int[size * size];
      images.add(buf);
    }

    s.nextInt();
    int row = s.nextInt(16);
    int offset = (i / 2);
    if (i % 2 == 0) {
      for (int j = 0; j < size; j++) {
        buf[j * size + offset] |= (row >>> j) & 1;
      }
    } 
    else {
      row <<= 1;
      for (int j = 0; j < size; j++) {
        buf[j * size + offset] |= (row >>> j) & 2;
      }
    }

    i++;
    if (i == size * 2) {
      i = 0;
    }
  }
}

