#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <list>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <time.h>
//#include <functional>

#include "of.h"
#include "ofOffPointsReader.h"
#include "Handler.hpp"
#include "GL_Interactor.h"
#include "ColorRGBA.hpp"
#include "Cores.h"
#include "Point.hpp"
#include "printof.hpp"


#include "CommandComponent.hpp"
#include "MyCommands.hpp"

#include "ofVertexStarIteratorSurfaceVertex.h"


clock_t start_insert;
clock_t end_insert;
clock_t start_print;
clock_t end_print;


using namespace std;
using namespace of;

//Define o tamanho da tela.
scrInteractor *Interactor = new scrInteractor(800, 600);

//Define a malha a ser usada.
typedef of::MyofDefault2D TTraits;
typedef of::ofMesh<TTraits> TMesh;
TMesh *malha;
Handler<TMesh> meshHandler;
int idCelulaInicio = NULL;

typedef PrintOf<TTraits> TPrintOf;

TPrintOf *Print;

typedef MyCommands<TPrintOf> TMyCommands;
typedef CommandComponent TAllCommands;

ofVtkWriter<TTraits> writer;
TAllCommands *allCommands;

//##################################################################//

////////////////////////////////////////////////////////////////////////
int type = 3;
//CASO 1 EXECUTA CRUST
//CASO 2 EXECUTA BETA-SKELETON
//CASO 3 EXECUTA ARVORE
////////////////////////////////////////////////////////////////////////

double coordenadasBaricentricas(double pDouble[3], ofMyCell<MyofDefault2D::sTraits> *pCell);

void RenderScene(void) {
    allCommands->Execute();
    Print->Vertices(malha, blue, 3);
    Print->FacesWireframe(malha, grey, 3);

    if (idCelulaInicio) {
        Print->Face(malha->getCell(idCelulaInicio), red);
    }

    glFinish();
    glutSwapBuffers();
}

void HandleKeyboard(unsigned char key, int x, int y) {


    double coords[3];
    char *xs[10];
    allCommands->Keyboard(key);

    switch (key) {
        case 'e':
            exit(1);
            break;

        case 'v':
            coords[0] = x;
            coords[1] = -y;
            coords[2] = 0.0;
            malha->addVertex(coords);
            break;

        case 's':
            break;

        case 'd':
            break;
    }


    Interactor->Refresh_List();
    glutPostRedisplay();
}


double areaTriangulo(double a[2], double b[2], double c[2]) {
    /* Area dada por metade do determinante da matriz:
     *
     * | a[0]  a[1]  1 |
     * | b[0]  b[1]  1 |
     * | c[0]  c[1]  1 |
     *
     */
    double area = 0.5d * (a[0]*b[1] + b[0]*c[1] + c[0]*a[1] - b[1]*c[0] - c[1]*a[0] - a[1]*b[0]);
    return area;
}


void coordenadasBaricentricas(double p[2], ofMyCell<MyofDefault2D::sTraits> *pCell, double bar[3]) {
    double *p1, *p2, *p3;
    p1 = malha->getVertex(pCell->getVertexId(0))->getCoords();
    p2 = malha->getVertex(pCell->getVertexId(1))->getCoords();
    p3 = malha->getVertex(pCell->getVertexId(2))->getCoords();

    double areaCell = areaTriangulo(p, p2, p3);
    bar[0] = areaTriangulo(p, p2, p3) / areaCell;
    bar[1] = areaTriangulo(p1, p, p3) / areaCell;
    bar[2] = areaTriangulo(p1, p2, p) / areaCell;
}


int encontraCelulaClicada(int x, int y) {
    int i;
    double coords[3];
    double bar[3];
    auxPoint<int> auxPoint;

    // encontra as coordenadas do ponto
    auxPoint.setValues(x, y);
    Interactor->ScreenToPoint(auxPoint, coords);
    coords[2] = 0.0;

    // varre todas celulas
    for (i = 0; i < malha->getNumberOfCells(); i++) {
        coordenadasBaricentricas(coords, malha->getCell(i);, bar);

        // se as coordenadas baricentricas sao todas positivas
        if (bar[0] >=0 && bar[1] >=0 && bar[2] >=0) {
            return i;
        }
    }
    return 0;
}

void HandleMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        idCelulaInicio = encontraCelulaClicada(x, y);
        Interactor->Refresh_List();
    }
}

using namespace std;

int main(int *argc, char **argv) {

    ofRuppert2D<MyofDefault2D> ruppert;
    ofPoints2DReader<MyofDefault2D> reader;
    ofVtkWriter<MyofDefault2D> writer;
    Interactor->setDraw(RenderScene);
    meshHandler.Set(new TMesh());
    char *fileBrasil = "/home/rafael/Bia/MVGA/ep_meu/Brasil.off";


    reader.readOffFile(fileBrasil);

    ruppert.execute2D(reader.getLv(), reader.getLids(), true);
    //writer.write(ruppert.getMesh(),"out.vtk",reader.getNorma(),ruppert.getNumberOfInsertedVertices());

    meshHandler = ruppert.getMesh();
    malha = ruppert.getMesh();


    Print = new TPrintOf(meshHandler);

    allCommands = new TMyCommands(Print, Interactor);

    double a, x1, x2, y1, y2, z1, z2;

    of::ofVerticesIterator<TTraits> iv(&meshHandler);

    iv.initialize();
    x1 = x2 = iv->getCoord(0);
    y1 = y2 = iv->getCoord(1);
    z1 = z2 = iv->getCoord(2);

    for (iv.initialize(); iv.notFinish(); ++iv) {
        if (iv->getCoord(0) < x1) x1 = a = iv->getCoord(0);
        if (iv->getCoord(0) > x2) x2 = a = iv->getCoord(0);
        if (iv->getCoord(1) < y1) y1 = a = iv->getCoord(1);
        if (iv->getCoord(1) > y2) y2 = a = iv->getCoord(1);
        if (iv->getCoord(2) < z1) z1 = a = iv->getCoord(2);
        if (iv->getCoord(2) > z2) z2 = a = iv->getCoord(2);
    }

    double maxdim;
    maxdim = fabs(x2 - x1);
    if (maxdim < fabs(y2 - y1)) maxdim = fabs(y2 - y1);
    if (maxdim < fabs(z2 - z1)) maxdim = fabs(z2 - z1);

    maxdim *= 0.6;

    Point center((x1 + x2) / 2.0, (y1 + y2) / 2.0, (y1 + y2) / 2.0);
    Interactor->Init(center[0] - maxdim, center[0] + maxdim,
                     center[1] - maxdim, center[1] + maxdim,
                     center[2] - maxdim, center[2] + maxdim, argc, argv);


    AddKeyboard(HandleKeyboard);
    AddMouse(HandleMouse);

	allCommands->Help(std::cout);
	std::cout<< std::endl<< "Press \"?\" key for help"<<std::endl<<std::endl;
	double t;

	Init_Interactor();

    return EXIT_SUCCESS;
}
