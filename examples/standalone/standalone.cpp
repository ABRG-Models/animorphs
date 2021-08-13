#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <src/visual/scene.h>
#include <src/softmatsim.h>
#include <src/core/animat.h>
#include <src/collisions/collision.h>
#include <src/collisions/collisionconstr.h>
#include <morph/Config.h>

using namespace morph::softmats;

int main( int n, char** args ){

    // GENERAL SETUP
    if(n<2){
        std::cerr << "Usage: " << args[0] << " /path/to/params.json [/path/to/logdir]" << std::endl;
        return 1;
    }

    int fps = 30;
    morph::Config conf(args[1]);
    std::string sceneFiles = conf.getString("pathToSceneFiles", "");
    Scene S(conf.getString("vertexShaderPath", "NULL"), conf.getString("fragmentShaderPath", "NULL"));

    PBD *solver = new PBD();
    BodySet *animats = new BodySet();

        // load textures
    const Json::Value tx = conf.getArray ("textures");
    for (int i=0; i<tx.size(); i++) {
        std::stringstream st;
        st<<sceneFiles<<tx[i].asString();
        S.addTexture(st.str());
    }

    {   // add the ground
        Ground *g = new Ground( -2.0 );
        g->type = BodyType::GROUND;
        animats->add(g);
        animats->reset();
    }

        // add animats
    const Json::Value mx = conf.getArray ("meshes");
    for (int i=0; i<mx.size(); i++) {
        std::stringstream st;
        st<<sceneFiles<<mx[i].asString();
        Animat* A = new Animat(st.str(), 0., i*5, 0.);
        A->setMass( 100.0 );
        A->setConstraints();
        A->type = BodyType::ANIMAT;
        animats->add(A);
    }

        // associate textures with animats
    int k=0;
    for( Body *b : animats->getBodies() ){
        S.addSceneObject(sceneObject(S.programID,b->getMesh()));
        if( b->type == BodyType::ANIMAT ){
            S.pairObjectTexture(S.mySceneObjects.size()-1,1+(k%(tx.size()-1)));
            k++;
        }
    }

    std::stringstream st;
    st<<sceneFiles<<"cone.obj";

    S.addScenePrimitive(st.str());

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

    do {


        // INTEGRATE THE MODEL
        animats->resetForces();
        animats->resetReceptors();
        solver->loop( animats, step );

        if(animats->hasContacts()){
            ContactList *contacts = animats->getContacts();
            contacts->print();
            std::cout << "Contact area : " << contacts->getContactArea( false ) << "\n";
        }

        for( Body *b : animats->getBodies() ){
            if( b->type == BodyType::ANIMAT ){
                b->getMesh()->updateVertexNormals();
            }
        }


        // UPDATE THE DISPLAY
        if(step++ % fps == 0){ continue; }

        int k=0;
        for( Body *b : animats->getBodies() ){
            S.mySceneObjects[k].updatePositions(b->getMesh());
            k++;
        }

        S.update();


        } while( S.running );

    return 0;
}

