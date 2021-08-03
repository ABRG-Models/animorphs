#pragma once

#include <iostream>
#include <vector>
#include "body.h"
#include "point.h"
#include "face.h"

namespace morph{ namespace softmats{
/**
 * Represets an Animat body
 *
 * @author Alejandro Jimenez Rodriguez
 */
class Animat : public Body{
private:
    void init(int);
    void init(std::string, int);
public:
    Animat( float x, float y, float z );
    Animat( std::string, float x, float y, float z );

    void setMass( double m );
    void setConstraints();
    void addGroundImpulse( arma::vec f );
    void move( double x, double y, double z );

};

}}
