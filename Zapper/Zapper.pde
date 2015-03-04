/*
* ---------- Interfaces that make conversion to mobile simpler ----------
*/
// Interfaces documented in InterfacesExample
interface PLabBridge {
  public int getWidth ();
  public int getHeight ();
  public void write (String string);
  public void subscribeRead (PLabRead sub);
  public void subscribeError (PLabRead sub);
  public void disconnect();
}
interface PLabRead {
  public void read(String string);
}


/*
 * ---------- Binding the code with call javascript/serial port ----------
 */
private PLabBridge pBridge;
private String received = null;

void bindPLabBridge (PLabBridge bridge) {
  size(bridge.getWidth(), bridge.getHeight());
  pBridge = bridge;
  
  // Subscribe to messages. Print incomming messages and change color of drawing
  bridge.subscribeRead(new PLabRead() {
    public void read (String string) {
     //  btRead(string.substring(0,string.length()-2));
    }
  });
}

void btWrite(String string) {
  if (pBridge != null) {
    pBridge.write(string);
  }
}

//-------------
//
//  Minimal Processing GUI library.
//  PLab 2015
//
//  Dag Svanæs, IDI, NTNU.
// 

int[] white = {255,255,255};
int[] black = {0,0,0};
int[] red = {255,0,0};
int[] green = {0,255,0};
int[] blue = {0,0,255};
int[] yellow = {255,255,0};
int[] gray = {128,128,128};

float scaleOfGUI = 1.0;
int canvasWidth = 0, canvasHeight = 0;

void setCanvas(int w, int h) {
  canvasWidth = w;
  canvasHeight = h;
  size(w,h);
}

void scaleGUI() {
   if (pBridge != null) {
     scaleOfGUI = pBridge.getWidth() / canvasWidth;
     scale(scaleOfGUI);
   };
}

boolean inside(int x,int y, int x0,int y0,int w,int h) {
  return (((x >= x0) && (x < (x0+w))) && 
      ((y >= y0) && (y < (y0+h))));
}

boolean mouseInside(int[] rect) {
  int mouseXScaled = int(mouseX / scaleOfGUI);
  int mouseYScaled = int(mouseY / scaleOfGUI);
  return inside(mouseXScaled, mouseYScaled, rect[0],rect[1],rect[2],rect[3]);
}

void drawButton(int[] xywh, String buttonText) {
    boolean mousePressedInButton = mouseInside(xywh) && mousePressed;
    stroke(0);
    if (mousePressedInButton) {
      fill(0); }
    else {
      fill(255); 
    } 
    rect(xywh[0],xywh[1],xywh[2],xywh[3],10);
    if (mousePressedInButton) {
      fill(255); }
    else {
      fill(0); 
    }   
    int tSize = xywh[3] * 3 / 5;
    textSize(tSize);
    textAlign(CENTER, CENTER);
    text(buttonText,xywh[0]+xywh[2]/2,xywh[1]+xywh[3]/2-2);
}

void drawText(int[] xywh, String buttonText) {
    stroke(0);
    fill(255);  
    rect(xywh[0],xywh[1],xywh[2],xywh[3]);
    fill(0);  
    int tSize = xywh[3] * 3 / 5;
    textSize(tSize);
    textAlign(LEFT, CENTER);
    text(buttonText,xywh[0]+2,xywh[1]+xywh[3]/2-2);
}
  
void drawRectangle(int[] xywh, int[] drawColor) {
    stroke(0);
    fill(drawColor[0],drawColor[1],drawColor[2]);  
    rect(xywh[0],xywh[1],xywh[2],xywh[3]);
}

void drawCircle(int[] xyr, int[] drawColor) {
    stroke(0);
    fill(drawColor[0],drawColor[1],drawColor[2]);  
    ellipseMode(RADIUS);
    ellipse(xyr[0],xyr[1],xyr[2],xyr[2]);
}

/*
* ---------- End of library  -------------
*/
//------------------------------------------------------------------------------------- 
//-------------------------------------------------

int[] buttonFrame1 = {70,10,100,30};
String buttonString1 = "FIGHT!";

int[] buttonFrame2 = {120,50,100,30};
String buttonString2 = "Turn Right";

int[] buttonFrame3 = {10,50,100,30};
String buttonString3 = "Turn Left";

int[] buttonFrame4 = {70,120,100,40};
String buttonString4 = "STOP!";

boolean bluetoothActive = false;
void setup() {
  setCanvas(400,600);              // Canvas size is 200 x 200 pixels.
  background(128);            // Background color is gray (128,128,128).
  stroke(0);                  // Stroke color is black (0,0,0)
}

void btRead(String string) {
  println("Received: " + string);
}
  
void draw() {
  drawButton(buttonFrame1,buttonString1);
  drawButton(buttonFrame2,buttonString2);
  drawButton(buttonFrame3,buttonString3);
  drawButton(buttonFrame4,buttonString4);
}

void mousePressed() {
   if (mouseInside(buttonFrame1)) {
      btWrite("FIGHT");
   } else 
   if (mouseInside(buttonFrame2)) {
      btWrite("TR");  
   } else
   if (mouseInside(buttonFrame3)){
     btWrite("TL");
   } else
   if (mouseInside(buttonFrame4)){
     btWrite("STOP");
   }
}
