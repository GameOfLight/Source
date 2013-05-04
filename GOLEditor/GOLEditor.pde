import java.awt.event.*;
import java.util.Scanner;

int size = 8;
ArrayList<int[]> images = new ArrayList<int[]>();
int[] buf = new int[size * size];
color[] colors = {
  #000000, 
  #00d020, 
  #ff0000, 
  #ffa000
};

int mX, mY;
int curColor = 1;
int curImage = 0;

boolean cursor = false;

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
}

void draw() {
  stroke(255);
  line(0, size * 20, width, size * 20);
  line(size * 20, 0, size * 20, height);
  line(size * 20, size * 10, width, size * 10);
  noStroke();
  fill(192);
  rect(0, height, width, -64);
  stroke(0);
  fill(255);
  rect(10, size * 20 + 10, width / 3, 20);
  rect(width - 10, size * 20 + 10, -width / 3, 20);
  fill(0);
  text("import", 10 + width / 6, size * 20 + 25);
  text("export", width - 10 - width / 6, size * 20 + 25);
  text("" + (curImage + 1), width / 2, size * 20 + 25);
}

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

void mousePressed() {
  int i = mX + mY * size;

  noStroke();
  if (mouseY < size * 20) {
    set(pmouseX, pmouseY, 0);
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
  else if (mouseY > size * 20 + 10 && mouseY < size * 20 + 30) {
    if (mouseX < width - 10 && mouseX > width - 10 - width / 3) {
      exportImages();
    } 
    else if (mouseX > 10 && mouseX < width / 3 + 10) { 
      importImages();
    }
  }
}

void mouseDragged() {
  mousePressed();
  mouseMoved();
}

void mouseMoved() {
  int x = mouseX / 20;
  int y = mouseY / 20;
  int i = mX + mY * size;
  noStroke();

  set(pmouseX, pmouseY, 0);
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

void mouseWheeled(int mouseZ) {
  curColor += mouseZ + 2;
  curColor %= 3;
  curColor++;
  noStroke();
  mouseMoved();
}

void keyReleased() {
  if (keyCode == LEFT) {
    switchImage(-1);
  } 
  else if (keyCode == RIGHT) {
    switchImage(1);
  } else if (keyCode == UP) {
    mouseWheeled(-1);
  } else if (keyCode == DOWN) {
    mouseWheeled(1);
  } 
  else if (key == '+') {
    addImage();
  }
}

void switchImage(int dir) {
  curImage += dir + images.size();
  curImage %= images.size();
  buf = images.get(curImage);
  drawBuffer();
}

void addImage() {
  buf = new int[size * size];
  curImage = images.size();
  images.add(buf);
  drawBuffer();
}

void exportImages() {
  selectInput("Select a file to export the sheet to:", "exportImages");
}

void exportImages(File f) {
  if (f == null) {
    return;
  }
  
  if (f.getName().endsWith(".png") || f.getName().endsWith(".jpg") || f.getName().endsWith(".tga") || f.getName().endsWith(".tif")) {
    exportToImage(f);
  } else {
    exportToCArray(f);
  }
}

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

void exportToCArray(File f) {
  int[] buf;
  PrintWriter pw = null;
  try {
    pw = new PrintWriter(f);
    pw.println("prog_uchar sheet[] PROGMEM = {");
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

void importImages() {
  selectInput("Select a file to import sprites from:", "importImages");
}

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

