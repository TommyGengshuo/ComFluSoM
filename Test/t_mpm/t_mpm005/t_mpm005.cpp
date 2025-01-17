/************************************************************************
 * ComFluSoM - Simulation kit for Fluid Solid Soil Mechanics            *
 * Copyright (C) 2019 Pei Zhang                                         *
 * Email: peizhang.hhu@gmail.com                                        *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * any later version.                                                   *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program. If not, see <http://www.gnu.org/licenses/>  *
 ************************************************************************/

// 3D simlation of soil with Mohr Coulomb model


#include <MPM.h>

int main(int argc, char const *argv[])
{
	// Size of one grid
	Vector3d gridSize (1,1,1);
	// Domain size
	int nx = 500;
	int ny = 500;
	int nz = 150;
	// Create MPM domain
	MPM* a = new MPM(/*shape function type*/3, nx, ny, nz, gridSize);
	// Initialization
	a->Init();
	// Physcial parameters of particles
	double rhosPhysical 	= 2039.435;			// Physical density, unit [kg/m^3]
	Vector3d GPhysical (0., 0., -9.8);			// Body force
	double YoungPhysical 	= 7.5e7;			// Young's modus, unit [kg/(m*s^2)] (or Pa)
	double PoissonPhysical  = 0.3;				// Possion ratio
	double CPhysical		= 0.0e3;			// Cohesion coefficient, unit [kg/(m*s^2)] (or Pa)
	double PhiPhysical		= 45./180.*M_PI;	// Angle of internal friction
	double PsiPhysical		= 0./180.*M_PI;		// Angle of dilatation
	// Space time and mass step
	double dx = 0.5;							// unit [m]
	double dt = 1.0e-4;							// unit [s]
	double dm = 1.0e-1;							// unit [kg]
	// How many particles in a cell
	double Ratio = 1./4.;
	// Dementionless parameters of particles
	Vector3d G 		= GPhysical*dt*dt/dx;
	double Mp 		= rhosPhysical*pow(dx,3)*pow(Ratio,3)/dm;
	double Young 	= YoungPhysical*dx*dt*dt/dm;
	double Poisson 	= PoissonPhysical;
	double Rhos 	= rhosPhysical/dm*dx*dx*dx;
	double C 		= CPhysical*dx*dt*dt/dm;
	double Phi 		= PhiPhysical;
	double Psi 		= PsiPhysical;
	double K0 		= Poisson/(1.-Poisson);
	double mu = tan(PhiPhysical);
	// Start point of the box for generating particles
	Vector3d x0 (240, 240, 20);
	// demention of the box
	Vector3d l0 (20, 4, 10);
	a->Nproc = 12;
	a->Dc = 0.;
	// Generate a box of particles
	a->AddBoxParticles(-1, x0, l0, Ratio, Mp);
	// Define Mohr Coulomb parameters and gravity
	for (size_t p=0; p<a->Lp.size(); ++p)
	{
		a->Lp[p]->SetDruckerPrager(0, Young, Poisson, Phi, Psi, C);
		a->Lp[p]->B  = G;
		// Init stress
		a->Lp[p]->Stress(2,2) = -(a->Lp[p]->X(2)-(x0(2)+l0(2)))*G(2)*Rhos;
		a->Lp[p]->Stress(1,1) = a->Lp[p]->Stress(2,2)*K0;
		a->Lp[p]->Stress(0,0) = a->Lp[p]->Stress(1,1);
	}
	// All following boundaries are set as free slipping BC
	// Define boundary
	for (int i=0; i<nx; ++i)
	for (int j=0; j<ny; ++j)
	for (int k=19; k<=20; ++k)
	{
		Vector3d norm (0., 0., -1.);
		// a->SetSlippingBC(i,j,k, norm);
		a->SetFrictionBC(i,j,k,mu,norm);
	}
	// Define boundary
	for (int i=0; i<nx; ++i)
	for (int j=239; j<=240; ++j)
	for (int k=0; k<nz; ++k)
	{
		Vector3d norm (0., -1., 0.);
		a->SetSlippingBC(i,j,k, norm);
	}
	// Define boundary
	for (int i=0; i<nx; ++i)
	for (int j=244; j<=245; ++j)
	for (int k=0; k<nz; ++k)
	{
		Vector3d norm (0., 1., 0.);
		a->SetSlippingBC(i,j,k, norm);
	}
	// Define boundary
	for (int i=239; i<=240; ++i)
	for (int j=0; j<ny; ++j)
	for (int k=0; k<nz; ++k)
	{
		Vector3d norm (-1., 0., 0.);
		a->SetNonSlippingBC(i,j,k);
	}
	// Solve
	a->SolveMUSL(/*total time step*/50000,/*save per x time step*/100);
	return 0;
}