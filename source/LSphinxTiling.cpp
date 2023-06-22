#include "pch.h"
#include "LSphinxTiling.h"

LSphinxTiling::LSphinxTiling()
{

}


LSphinxTiling::~LSphinxTiling()
{

}

bool LSphinxTiling::Initialize(ID3D11Device* device, float width, int iterations, float seed)
{
	// STEP 1: Write production rules...
	LProductionRuleType FClockwise;
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FAntiClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 0.0f; // NB: Needed on all rules but the first!
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FAntiClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FAntiClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = -DirectX::XM_PI/3.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = DirectX::XM_PI/3.0f;
		return KModule;
		});
	FClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
		});
	FClockwise.weight = 1.0f;
	AddProductionRule("FClockwise", FClockwise);

	LProductionRuleType FAntiClockwise;
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = DirectX::XM_PI/3.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = -DirectX::XM_PI/3.0f;
		return KModule;
		});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
		});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FAntiClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 2.0f*DirectX::XM_PI/3.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "FClockwise";
		KModule.staticLength = LModule.staticLength/2.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
	});
	FAntiClockwise.productions.push_back([](LModuleType LModule) {
		LModuleType KModule = LModule;
		KModule.letter = "";
		KModule.staticLength = LModule.staticLength/4.0f;
		KModule.staticRotation = 0.0f;
		return KModule;
	});
	FAntiClockwise.weight = 1.0f;
	AddProductionRule("FAntiClockwise", FAntiClockwise);

	// STEP 2: Write axiom...
	LModuleType FAntiClockwiseModule = LModuleType();
	FAntiClockwiseModule.letter = "FAntiClockwise";
	FAntiClockwiseModule.staticLength = 2.0f;
	FAntiClockwiseModule.staticWidth = width;

	std::vector<LModuleType> axiom = std::vector<LModuleType>{ FAntiClockwiseModule };

	// STEP 3: Initialise...
	return __super::Initialize(device, axiom, iterations, seed, 0.0f); // NB: Oriented to best 'tend towards a limit'...
}
