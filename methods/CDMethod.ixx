//
// Created by Муслихиддин Махмудов on 29.09.2026.
//


module;
#include <Eigen/Dense>

export module CDMethod;



using State = Eigen::Vector3d;

export inline constexpr auto CD = [](const State& s, double dt, auto sys, auto sys_) -> State {

    State k1;
    State tempS = s;

    //    for (int i = 0; i< s.size(); i++) {
    //        k1 = sys(tempS);
    //        tempS[i] += k1[i]*dt*0.5;
    //    }
    //
    //    for (int i = s.size()-1; i>=0; i--) {
    //        k1 = sys_(tempS);
    //        tempS[i] += k1[i]*dt*0.5;
    //    }

    tempS[0] += (tempS[1]*-0.2)*dt*0.5;
    tempS[1] += (tempS[0]+tempS[2])*dt*0.5;
    tempS[2] = (tempS[2]+dt*0.5*(tempS[0]+tempS[1]*tempS[1]))/(1+dt*0.5);


    tempS[2] += (tempS[0]+tempS[1]*tempS[1]-tempS[2])*dt*0.5;
    tempS[1] += (tempS[0]+tempS[2])*dt*0.5;
    tempS[0] += (tempS[1]*-0.2)*dt*0.5;




    return tempS;
};
