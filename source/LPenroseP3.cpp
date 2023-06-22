#include "pch.h"
#include "LPenroseP3.h"

LPenroseP3::LPenroseP3()
{

}


LPenroseP3::~LPenroseP3()
{

}

bool LPenroseP3::Initialize(ID3D11Device* device, float width, int iterations, float seed)
{
	// STEP 0: Define any 'common' productions...
	std::function<LModuleType(LModuleType)> PushProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "[";
		return KModule;
	};
	std::function<LModuleType(LModuleType)> PopProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "]";
		return KModule;
	};

	std::function<LModuleType(LModuleType)> AxiomAProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "A";
		KModule.staticLength = 1.0f;
		//KModule.staticRotation += 72.0f;
		return KModule;
	};
	std::function<LModuleType(LModuleType)> AxiomThetaProduction = [](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "X";
		KModule.staticRotation += 72.0f * (DirectX::XM_PI/180.0f);
		return KModule;
	};

	// STEP 1: Write production rules...
	LProductionRuleType Axiom;
	//Axiom.productions.push_back(AxiomAProduction);
	//Axiom.productions.push_back(AxiomAProduction);
	/*Axiom.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "A";
		KModule.staticLength = 0.0f;
		KModule.staticRotation += 0.0f;
		return KModule;
		});*/
	for (int i = 0; i < 5; i++)
	{
		Axiom.productions.push_back([](LModuleType LModule) {
			LModuleType KModule = LModule;
			KModule.letter = "[";
			return KModule;
			});
		Axiom.productions.push_back(AxiomAProduction);
		Axiom.productions.push_back([](LModuleType LModule) {
			LModuleType KModule = LModule;
			KModule.letter = "]";
			//KModule.staticRotation += 72.0f * (DirectX::XM_PI/180.0f);
			return KModule;
			});
		Axiom.productions.push_back(AxiomThetaProduction);
	}
	Axiom.weight = 1.0f;
	AddProductionRule("Axiom", Axiom);

	// STEP 2: Write axiom...
	LModuleType AxiomModule = LModuleType();
	AxiomModule.letter = "Axiom";
	AxiomModule.staticLength = 1.0f;
	AxiomModule.staticWidth = width;

	std::vector<LModuleType> axiom = std::vector<LModuleType>{ AxiomModule };

	// STEP 3: Initialise...
	return __super::Initialize(device, axiom, iterations, seed, DirectX::XM_PIDIV2);
}
