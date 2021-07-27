#include <src/softmatsim.h>
#include <src/core/animat.h>
#include <src/collisions/collision.h>
#include <src/collisions/collisionconstr.h>
#include <src/util/config.h>

using namespace morph::softmats;

int main( int n, char** args ){

    // GENERAL SETUP
    if(n<2){
        std::cerr << "Usage: " << args[0] << " /path/to/params.json [/path/to/logdir]" << std::endl;
        return 1;
    }

    Config::configPath = args[1];

    int fps = 20;

    SoftmatsView *view = new SoftmatsView();
    PBD *solver = new PBD();
    BodySet *animats = new BodySet();


    // MODEL SPECIFIC SETUP
    {
        // add the ground
        Ground *g = new Ground( -2.0 );
        g->type = BodyType::GROUND;
        animats->add(g);
        animats->reset();
        view->setupGround(g);
    }

    {
        // add an animat
        Animat* a = new Animat(0., 2., 0.);
        a->setMass( 100.0 );
        a->setConstraints();
        a->type = BodyType::ANIMAT;
        animats->add(a);
    }

    {
        // add an animat
        Animat* b = new Animat(0., 10., 0.);
        b->setMass( 100.0 );
        b->setConstraints();
        b->type = BodyType::ANIMAT;
        animats->add(b);
    }

    // GENERAL SETUP
    animats->reset();
    animats->addExternalForce(arma::vec {0.0, -10.0, 0.0});

    {
        CollisionConstraint *cc = new CollisionConstraint();
        cc->setCollisionTest(new ContinuousCollisionTest());
        animats->addCollisionConstraint( cc );
    }

    // MAIN PROGRAM LOOP
    int step = 0;
    bool running = true;

    while( running && !view->shouldClose() ){


        // INTEGRATE THE MODEL

        animats->resetForces();

        animats->resetReceptors();
        solver->loop( animats, step );

        if(animats->hasContacts()){
            ContactList *contacts = animats->getContacts();
            contacts->print();
            std::cout << "Contact area : " << contacts->getContactArea( false ) << "\n";
        }



        // UPDATE THE DISPLAY

        if( step++ % fps == 0 ){
            continue;
        }

        for( Body *b : animats->getBodies() ){
            if( b->type == BodyType::ANIMAT ){
                b->getMesh()->updateVertexNormals();
            }
        }

        view->preDisplay();

        view->displayGround();
        for( Body *b : animats->getBodies() ){
            if( b->type == BodyType::ANIMAT ){
                view->displayBody( b );
            }
        }

        view->updateCamera();

        view->postDisplay();

    }
    return 0;
}
