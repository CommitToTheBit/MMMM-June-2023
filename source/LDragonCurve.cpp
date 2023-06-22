#include "pch.h"
#include "LDragonCurve.h"

LDragonCurve::LDragonCurve()
{

}


LDragonCurve::~LDragonCurve()
{

}

bool LDragonCurve::Initialize(ID3D11Device* device, float width, int iterations, float seed)
{
	// STEP 1: Write production rules...
	LProductionRuleType F;
	F.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "F";
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	F.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "+";
		KModule.staticLength = 0.0f;
		KModule.staticRotation = 90.0f*DirectX::XM_PI/180.0f;
		return KModule;
		});
	F.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "G";
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	F.weight = 1.0f;
	AddProductionRule("F", F);

	LProductionRuleType G;
	G.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "F";
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	G.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "-";
		KModule.staticLength = 0.0f;
		KModule.staticRotation = -90.0f*DirectX::XM_PI/180.0f;
		return KModule;
		});
	G.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "G";
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	G.weight = 1.0f;
	AddProductionRule("G", G);

	// STEP 2: Write axiom...
	LModuleType FModule = LModuleType();
	FModule.letter = "F";
	FModule.staticLength = 1.0f;
	FModule.staticWidth = width;

	std::vector<LModuleType> axiom = std::vector<LModuleType>{ FModule };

	// STEP 3: Initialise...
	return __super::Initialize(device, axiom, iterations, seed, -iterations*DirectX::XM_PIDIV4); // NB: Oriented to best 'tend towards a limit'...
}
