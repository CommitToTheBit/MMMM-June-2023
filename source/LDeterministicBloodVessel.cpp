#include "pch.h"
#include "LDeterministicBloodVessel.h"

LDeterministicBloodVessel::LDeterministicBloodVessel()
{

}


LDeterministicBloodVessel::~LDeterministicBloodVessel()
{

}

bool LDeterministicBloodVessel::Initialize(ID3D11Device* device, float width, float asymmetry, int iterations, float seed)
{
	// STEP 0: Define any 'common' productions...
	std::function<LModuleType(LModuleType)> XProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "X";
		return KModule;
	};

	// STEP 1: Write production rules...
	LProductionRuleType C;
	C.productions.push_back(XProduction);
	C.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "[";
		return KModule;
		});
	C.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "C";
		KModule.staticLength *= 1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticRotation = acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+1.0f-pow(LModule.asymmetry, 4.0f))/(2.0f*pow(LModule.asymmetry, 0.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.staticWidth *= 1.0f/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		return KModule;
		});
	C.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "]";
		return KModule;
		});
	C.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "C";
		KModule.staticLength *= LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		KModule.staticRotation = -acos((pow(1.0f+pow(LModule.asymmetry, 3.0f), 4.0f/3.0f)+pow(LModule.asymmetry, 4.0f)-1.0f)/(2.0f*pow(LModule.asymmetry, 2.0f)*pow(1.0f+pow(LModule.asymmetry, 3.0f), 2.0f/3.0f)));
		KModule.staticWidth *= LModule.asymmetry/pow(1.0f+pow(LModule.asymmetry, 3.0f), 1.0f/3.0f);
		return KModule;
		});
	C.weight = 1.0f;
	AddProductionRule("C", C);

	// STEP 2: Write axiom...
	LModuleType CModule;
	CModule.letter = "C";
	CModule.staticLength = 1.0f;
	CModule.staticRotation = 0.0f;
	CModule.staticWidth = width;
	CModule.staticAsymmetry = asymmetry;

	std::vector<LModuleType> axiom = std::vector<LModuleType>{ CModule };

	// STEP 3: Initialise...
	return __super::Initialize(device, axiom, iterations, seed, DirectX::XM_PIDIV2);
}
