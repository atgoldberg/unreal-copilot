// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilot.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnrealCopilotPythonExecutionTest, "UnrealCopilot.PythonExecution.BasicTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUnrealCopilotPythonExecutionTest::RunTest(const FString& Parameters)
{
	FString OutResult;
	bool bResult;

	// Test 1: Valid Python code execution
	TestTrue("Valid Python Code - Simple Print", UUnrealCopilotBlueprintLibrary::ExecutePythonString(TEXT("print('Hello from Python')"), OutResult));
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 1 Result: %s"), *OutResult);

	// Test 2: Valid Python code - Simple math
	TestTrue("Valid Python Code - Math", UUnrealCopilotBlueprintLibrary::ExecutePythonString(TEXT("result = 2 + 2"), OutResult));
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 2 Result: %s"), *OutResult);

	// Test 3: Multi-line Python script
	FString MultiLineScript = TEXT(
		"x = 10\n"
		"y = 20\n"
		"print(f'Sum: {x + y}')"
	);
	TestTrue("Multi-line Python Script", UUnrealCopilotBlueprintLibrary::ExecutePythonString(MultiLineScript, OutResult));
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 3 Result: %s"), *OutResult);

	// Test 4: Invalid Python code (should return false)
	bResult = UUnrealCopilotBlueprintLibrary::ExecutePythonString(TEXT("prit('typo in print')"), OutResult);
	TestFalse("Invalid Python Code - Typo", bResult);
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 4 Result (Expected Failure): %s"), *OutResult);

	// Test 5: Empty string handling
	bResult = UUnrealCopilotBlueprintLibrary::ExecutePythonString(TEXT(""), OutResult);
	TestFalse("Empty String Input", bResult);
	TestTrue("Empty String Error Message", OutResult.Contains(TEXT("empty")));
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 5 Result: %s"), *OutResult);

	// Test 6: Unreal Engine Python integration
	TestTrue("Unreal Python Integration", UUnrealCopilotBlueprintLibrary::ExecutePythonString(TEXT("import unreal; print('Unreal Engine Python integration working')"), OutResult));
	UE_LOG(LogUnrealCopilot, Log, TEXT("Test 6 Result: %s"), *OutResult);

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS