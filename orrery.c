/*
==========================================================================
File:        orrery.c (skeleton)
Authors:     Toby Howard (skeleton), Costanza Maria Improta
==========================================================================
*/

/* The following ratios are not to scale: */
/* Moon orbit : planet orbit */
/* Orbit radius : body radius */
/* Sun radius : planet radius */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BODIES 20
#define TOP_VIEW 1
#define ECLIPTIC_VIEW 2
#define SHIP_VIEW 3
#define EARTH_VIEW 4
#define FLY_AROUND_VIEW 5
#define PI 3.14159
#define DEG_TO_RAD 0.017453293
#define ORBIT_POLY_SIDES 40
#define TIME_STEP 0.5   /* days per frame */
#define MAX_STARS 1000
#define SOL_SYS_SIZE 300000000
#define TURN_ANGLE 4.0
#define RUN_SPEED  1500000
#define TURN_ANGLE 4.0

typedef struct {
  char    name[20];       /* name */
  GLfloat r, g, b;        /* colour */
  GLfloat orbital_radius; /* distance to parent body (km) */
  GLfloat orbital_tilt;   /* angle of orbit wrt ecliptic (deg) */
  GLfloat orbital_period; /* time taken to orbit (days) */
  GLfloat radius;         /* radius of body (km) */
  GLfloat axis_tilt;      /* tilt of axis wrt body's orbital plane (deg) */
  GLfloat rot_period;     /* body's period of rotation (days) */
  GLint   orbits_body;    /* identifier of parent body */
  GLfloat spin;           /* current spin value (deg) */
  GLfloat orbit;          /* current orbit value (deg) */
 } body;


GLfloat starX, starY, starZ;  /*Star coordinates*/

body  bodies[MAX_BODIES];
int   numBodies, current_view, draw_labels, draw_orbits, draw_starfield;
int TOGGLE_AXES = 1;
float date;

// Coordinates for starship view
GLdouble lat,     lon;              /* View angles (degrees)    */
GLfloat  eyex,    eyey,    eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx,     upy,     upz;     /* View up vector           */
GLdouble dir_x, dir_y, dir_z;

/*****************************/

float myRand (void)
{
  /* return a random float in the range [0,1] */
  return (float) rand() / RAND_MAX;
}

/*****************************/

void drawStarfield (void)
{
  srand(1);

  // Get coordinates for 1000 stars
  int noStars;
  for(noStars = 0; noStars < MAX_STARS; noStars++){
    starX = myRand() * (SOL_SYS_SIZE*2) - SOL_SYS_SIZE;
    starY = myRand() * (SOL_SYS_SIZE*2) - SOL_SYS_SIZE;
    starZ = myRand() * (SOL_SYS_SIZE*2) - SOL_SYS_SIZE;
    glBegin(GL_POINTS);
      glColor3f(1.0, 1.0, 1.0);
      glVertex3f (starX, starY, starZ);
    glEnd();
  }

}

/*****************************/

void readSystem(void)
{
  /* reads in the description of the solar system */

  FILE *f;
  int i;

  f= fopen("sys", "r");
  if (f == NULL) {
     printf("ex2.c: Couldn't open the datafile 'sys'\n");
     printf("To get this file, use the following command:\n");
     printf("  cp /opt/info/courses/COMP27112/ex2/sys .\n");
     exit(0);
  }
  fscanf(f, "%d", &numBodies);
  for (i= 0; i < numBodies; i++)
  {
    fscanf(f, "%s %f %f %f %f %f %f %f %f %f %d",
      bodies[i].name,
      &bodies[i].r, &bodies[i].g, &bodies[i].b,
      &bodies[i].orbital_radius,
      &bodies[i].orbital_tilt,
      &bodies[i].orbital_period,
      &bodies[i].radius,
      &bodies[i].axis_tilt,
      &bodies[i].rot_period,
      &bodies[i].orbits_body);

    /* Initialise the body's state */
    bodies[i].spin= 0.0;
    bodies[i].orbit= myRand() * 360.0; /* Start each body's orbit at a
                                          random angle */
    bodies[i].radius*= 1000.0; /* Magnify the radii to make them visible */
  }
  fclose(f);
}

/*****************************/

void drawString (void *font, float x, float y, char *str)
{
  /* Displays the string "str" at (x,y,0), using font "font" */
  /* Taken from OpenGL manual*/
  char *ch;
  glRasterPos3f(x, y, 0.0);
  for (ch=str; *ch; ch++)
    glutBitmapCharacter(font, (int) *ch);
}

/*****************************/
void calculate_lookpoint(void)
{
  /* Given an eyepoint and latitude and longitude angles,
  will compute a look point one unit away */

  // Check if it is within the constraints
  if (lat > 90)
    lat = 89.99;
  if (lat < -90)
    lat = -89.99;

  GLdouble dir_x = cos(lat * DEG_TO_RAD) * sin(lon * DEG_TO_RAD)*100000000;
  GLdouble dir_y = sin(lat * DEG_TO_RAD)*100000000;
  GLdouble dir_z = cos(lat * DEG_TO_RAD) * cos(lon * DEG_TO_RAD)*100000000;

  centerx = eyex + dir_x;
  centery = eyey + dir_y;
  centerz = eyez + dir_z;
  //glutPostRedisplay();

} // calculate_lookpoint()

/*****************************/

void setView (void) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  switch (current_view) {
  case TOP_VIEW:
    gluLookAt(0.0001, 550000.0 *1000, 0.0001,
              0.0, 0.0, 0.0,
              0.0, 0.0, -1.0);

    break;
  case ECLIPTIC_VIEW:
    gluLookAt(0.1, 0.1, 550000.0 * 1000,
              0.0, 0.0, 0.0, // It looks towards the Sun
              0.0, 1.0, 0.0);
    break;
  case SHIP_VIEW:
    gluLookAt(50000.0 * 1000, 100000.0 * 1000, 300000.0 * 1000,
              0.0, 0.0, 0.0, // It looks towards the Sun
              0.0, 1.0, 0.0);
    break;

  case EARTH_VIEW:
    gluLookAt(bodies[3].orbital_radius * cos(bodies[3].orbit * DEG_TO_RAD),bodies[3].radius * 1.1,bodies[3].orbital_radius * sin(bodies[3].orbit * DEG_TO_RAD),
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    break;

  case FLY_AROUND_VIEW:
    calculate_lookpoint();
    gluLookAt(eyex, eyey, eyez,
              centerx, centery, centerz,
              upx, upy, upz);
    break;
  }
}

/*****************************/

void menu (int menuentry) {
  switch (menuentry) {
  case 1: current_view= TOP_VIEW;
          break;
  case 2: current_view= ECLIPTIC_VIEW;
          break;
  case 3: current_view= SHIP_VIEW;
          break;
  case 4: current_view= EARTH_VIEW;
          break;
  case 5: current_view= FLY_AROUND_VIEW;
          break;
  case 6: draw_labels= !draw_labels;
          break;
  case 7: draw_orbits= !draw_orbits;
          break;
  case 8: draw_starfield= !draw_starfield;
          break;
  case 9: exit(0);
  }
}

/*****************************/

void init(void)
{
  /* Define background colour */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutCreateMenu (menu);
  glutAddMenuEntry ("Top view", 1);
  glutAddMenuEntry ("Ecliptic view", 2);
  glutAddMenuEntry ("Spaceship view", 3);
  glutAddMenuEntry ("Earth view", 4);
  glutAddMenuEntry ("Fly Around view", 5);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Toggle labels", 6);
  glutAddMenuEntry ("Toggle orbits", 7);
  glutAddMenuEntry ("Toggle starfield", 8);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Quit", 9);
  glutAttachMenu (GLUT_RIGHT_BUTTON);

  current_view= TOP_VIEW;
  draw_labels= 1;
  draw_orbits= 1;
  draw_starfield= 1;

  lat = 0.0;
  lon = 0.0;

  eyex = 40000000.0;
  eyey = 10000000.0;
  eyez = -400000000.0;

  centerx = 0.0;
  centery = 0.0;
  centerz = 0.0;

  upx = 0.0;
  upy = 1.0;
  upz = 0.0;
}

/*****************************/

void animate(void)
{
  int i;

  date+= TIME_STEP;

    for (i= 0; i < numBodies; i++)  {
      bodies[i].spin += 360.0 * TIME_STEP / bodies[i].rot_period;
      bodies[i].orbit += 360.0 * TIME_STEP / bodies[i].orbital_period;
      glutPostRedisplay();
    }
}

/*****************************/


void draw_axes(void) {

  // Draws X Y and Z axis lines, of length AXES_LENGTH
  float AXES_LENGTH= 50000000.0;

  glLineWidth(3.0);

  // Draw x-axis
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0); // red
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(AXES_LENGTH, 0.0, 0.0);

  // Draw y-axis
  glColor3f(0.0,1.0,0.0); // green
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, AXES_LENGTH, 0.0);

  // Draw z-axis
  glColor3f(0.0,0.0,1.0); // blue
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, AXES_LENGTH);
  glEnd();
}

/*****************************/


void drawOrbit (int n)
{
  /* Draws a polygon to approximate the circular orbit of body "n" */

    float angleDiff = 360 / ORBIT_POLY_SIDES * DEG_TO_RAD;
    // Connect drawn points with a line
    glBegin(GL_LINE_LOOP);
    int orbitPoint;
    for(orbitPoint = 0; orbitPoint < ORBIT_POLY_SIDES; orbitPoint++)
    {
      glVertex3f(bodies[n].orbital_radius * sin(orbitPoint*angleDiff),
                 0.0,
                 bodies[n].orbital_radius * cos(orbitPoint*angleDiff));
    }
    glEnd();
}

/*****************************/

void drawLabel(int n)
{
  /* Draws the name of body "n" */
  glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
  drawString(GLUT_BITMAP_HELVETICA_18, 0.0,0.0,bodies[n].name);
}

/*****************************/

void drawBody(int n)
{
 /* Draws body "n" */
 /* This is for you to complete. */

  // If it orbits around the sun
  if (bodies[n].orbits_body == 0){
    glLineWidth(1.0);
    glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);

    // orbital tilt
    glRotatef(bodies[n].orbital_tilt ,0.0,0.0,1.0);

    // draw orbit of the planet
    if (draw_orbits)
      drawOrbit(n);

    // translation along the orbit
    glTranslatef(bodies[n].orbital_radius*cos(DEG_TO_RAD*bodies[n].orbit),
                0.0,
                bodies[n].orbital_radius*sin(DEG_TO_RAD*bodies[n].orbit));

    // axis tilt
    glRotatef(bodies[n].axis_tilt ,0.0,0.0,1.0);

    // spin around body's axis'
    glRotatef(bodies[n].spin * 0.01,0.0,1.0,0.0);

    // rotate sphere - otherwise poles horizontal
    glRotatef(90 ,1.0,0.0,0.0);

    if (draw_labels)
      drawLabel(n);

    // draw the axis
    glBegin(GL_LINES);
      glVertex3f(0.0, 0.0, -2 * bodies[n].radius);
      glVertex3f(0.0, 0.0, 2 * bodies[n].radius);
    glLineWidth(300.0);
    glEnd();

    glutWireSphere(bodies[n].radius, 10, 10);
  }
  else{
    glLineWidth(1.0);
    glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
    // get the planet the satellite orbits around
    GLint orbits_body = bodies[n].orbits_body;
    // rotate the body with parent's orbital tilt
    glRotatef(bodies[orbits_body].orbital_tilt ,0.0,0.0,1.0);
    // draw orbit of the planet
    glTranslatef(bodies[orbits_body].orbital_radius*cos(DEG_TO_RAD*bodies[orbits_body].orbit),
                0.0,
                bodies[orbits_body].orbital_radius*sin(DEG_TO_RAD*bodies[orbits_body].orbit));

    if (draw_orbits)
      drawOrbit(n);

    // translation along the orbit
    glTranslatef(bodies[n].orbital_radius*cos(DEG_TO_RAD*bodies[n].orbit),
                0.0,
                bodies[n].orbital_radius*sin(DEG_TO_RAD*bodies[n].orbit));

    // axis tilt
    glRotatef(bodies[n].axis_tilt ,0.0,0.0,1.0);

    // spin around body's axis'
    glRotatef(bodies[n].spin * 0.01,0.0,1.0,0.0);

    // rotate sphere - otherwise poles horizontal
    glRotatef(90 ,1.0,0.0,0.0);

    if (draw_labels)
      drawLabel(n);

    // draw the axis
    glBegin(GL_LINES);
      glVertex3f(0.0, 0.0, -2 * bodies[n].radius);
      glVertex3f(0.0, 0.0, 2 * bodies[n].radius);
    glLineWidth(300.0);
    glEnd();

    glutWireSphere(bodies[n].radius, 10, 10);
  }
}

/*****************************/

void display(void)
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);

  /* set the camera */
  setView();

  if (draw_starfield)
    drawStarfield();

  if (TOGGLE_AXES)
   draw_axes();

  for (i= 0; i < numBodies; i++)
  {
    glPushMatrix();
      drawBody (i);
    glPopMatrix();
  }

  glutSwapBuffers();
}

/*****************************/

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (48.0, (GLfloat) w/(GLfloat) h, 10000.0, 800000000.0);
}

/*****************************/

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27:  /* Esc */
      exit(0);
    case 97:  /* a */
      TOGGLE_AXES = !TOGGLE_AXES;
      break;
    case  117: /* u */
      eyey += 800000;
      break;
    case  100: /* d */
      eyey -= 800000;
      break;
    case  114: /* r */
      eyex -= 800000;
      break;
    case  108: /* l */
      eyex += 800000;
      break;
  }
}

/*****************************/
void cursor_keys(int key, int x, int y) {
  switch (key){
    if (current_view == FLY_AROUND_VIEW){
      case GLUT_KEY_UP:
        eyex = eyex + sin((lon) * DEG_TO_RAD) * RUN_SPEED;
        eyez = eyez + cos((lon) * DEG_TO_RAD) * RUN_SPEED;
        break;

      case GLUT_KEY_DOWN:
        eyex = eyex - sin((lon) * DEG_TO_RAD) * RUN_SPEED;
        eyez = eyez - cos((lon) * DEG_TO_RAD) * RUN_SPEED;
        break;

      case GLUT_KEY_LEFT:
        lon = lon + TURN_ANGLE;
        break;

      case GLUT_KEY_RIGHT:
        lon = lon -TURN_ANGLE;
        break;

      case GLUT_KEY_PAGE_UP:
        lat = lat + TURN_ANGLE;
        break;

      case GLUT_KEY_PAGE_DOWN:
        lat = lat - TURN_ANGLE;
        break;
    }
  }
}

/*****************************/


int main(int argc, char** argv)
{
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutCreateWindow ("COMP27112 Exercise 2");
  glutFullScreen();
  init ();
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutSpecialFunc (cursor_keys);
  glutIdleFunc (animate);
  readSystem();
  glutMainLoop ();
  return 0;
}
/* end of orrery.c */
