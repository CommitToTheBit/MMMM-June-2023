#include "pch.h"
#include "LBloodVessel.h"

#define MAJOR_BRANCH_DAMPING(x) 0.95f * x;
#define MINOR_BRANCH_DAMPING(x) 0.55f * x;
#define ANGLE_DAMPING(x) 1.25f * x;

LBloodVessel::LBloodVessel()
{

}


LBloodVessel::~LBloodVessel()
{

}

bool LBloodVessel::Initialize(ID3D11Device* device, float width, int iterations, float seed)
{
	// STEP 0: Define any 'common' productions...
	std::function<LModuleType(LModuleType)> XProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "X";
		return KModule;
	};

	// STEP 1: Write production rules...
	LProductionRuleType CForward;
	CForward.productions.push_back(XProduction);
	CForward.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "C";
		KModule.staticRotation			 =	 0.0f;
		KModule.staticWidth				*=	 1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		return KModule;
		});
	CForward.weight = 0.1f;
	AddProductionRule("C", CForward);

	LProductionRuleType CLeft;
	CLeft.productions.push_back(XProduction);
	CLeft.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "[";
		return KModule;
		});
	CLeft.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "L";
		KModule.staticLength			*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	 acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+1.0f-pow(LModule.asymmetry, 4.0f))/(2.0f*pow(LModule.asymmetry, 0.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.periodicRotation		*=	 1.0f+1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.randomPeriodicRotation	*=	 1.0f+1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticWidth				*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	CLeft.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "]";
		return KModule;
		});
	CLeft.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "R";
		KModule.staticLength			*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	-acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+pow(LModule.asymmetry, 4.0f)-1.0f)/(2.0f*pow(LModule.asymmetry, 2.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.periodicRotation		*=	 1.0f+1.0f*LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f); 
		KModule.randomPeriodicRotation	*=	 1.0f+1.0f*LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticWidth				*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	CLeft.weight = 0.45f;
	AddProductionRule("C", CLeft);

	LProductionRuleType CRight;
	CRight.productions.push_back(XProduction);
	CRight.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "[";
		return KModule;
		});
	CRight.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "L";
		KModule.staticLength			*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	 acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+pow(LModule.asymmetry, 4.0f)-1.0f)/(2.0f*pow(LModule.asymmetry, 2.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.periodicRotation		*=	 1.0f+1.0f*LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.randomPeriodicRotation	*=	 1.0f+1.0f*LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticWidth				*=	 MINOR_BRANCH_DAMPING(LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	CRight.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "]";
		return KModule;
		});
	CRight.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "R";
		KModule.staticLength			*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	-acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+1.0f-pow(LModule.asymmetry, 4.0f))/(2.0f*pow(LModule.asymmetry, 0.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.periodicRotation		*=	 1.0f+1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.randomPeriodicRotation	*=	 1.0f+1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticWidth				*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	CRight.weight = 0.45f;
	AddProductionRule("C", CRight);

	LProductionRuleType L;
	L.productions.push_back(XProduction);
	L.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "C";
		KModule.staticLength			*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	-ANGLE_DAMPING(acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+1.0f-pow(LModule.asymmetry, 4.0f))/(2.0f*pow(LModule.asymmetry, 0.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f))));
		KModule.staticWidth				*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	L.weight = 1.0f;
	AddProductionRule("L", L);

	LProductionRuleType R;
	R.productions.push_back(XProduction);
	R.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter					 =	 "C";
		KModule.staticLength			*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomStaticLength		*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.randomPeriodicLength	*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		KModule.staticRotation			 =	 ANGLE_DAMPING(acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+1.0f-pow(LModule.asymmetry, 4.0f))/(2.0f*pow(LModule.asymmetry, 0.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f))));
		KModule.staticWidth				*=	 MAJOR_BRANCH_DAMPING(1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f));
		return KModule;
		});
	R.weight = 1.0f;
	AddProductionRule("R", R);

	// STEP 2: Write axiom...
	LModuleType CModule;
	CModule.letter = "C";
	CModule.period = 2.5f;
	CModule.aperiodicity = 0.5f;
	CModule.synchronisation = 0.0f;
	CModule.asynchronicity = 1.0f;
	CModule.staticLength = 1.0f;
	CModule.randomStaticLength = 0.5f;
	CModule.randomPeriodicLength = 0.125f;
	CModule.staticRotation = 0.0f;
	CModule.randomStaticRotation = 4.0f*DirectX::XM_PI/180.0f;
	CModule.periodicRotation = 0.1f*DirectX::XM_PI/180.0f;
	CModule.randomPeriodicRotation = 1.0f*DirectX::XM_PI/180.0f;
	CModule.staticWidth = width;
	CModule.staticAsymmetry = 0.8f;
	CModule.randomStaticAsymmetry = 0.1f;

	std::vector<LModuleType> axiom = std::vector<LModuleType>{ CModule };

	// STEP 3: Initialise...
	return __super::Initialize(device, axiom, iterations, seed, DirectX::XM_PIDIV2);
}
