add_wn_library(WNScripting STATIC
  inc/WNArgList.h
  inc/WNArrayAccessExpression.h
  inc/WNArrayAllocation.h
  inc/WNAssignment.h
  inc/WNBinExpression.h
  inc/WNBuiltinTypeInitialization.h
  inc/WNCodeModule.h
  inc/WNCondExpression.h
  inc/WNConstantExpression.h
  inc/WNDeclList.h
  inc/WNDeclaration.h
  inc/WNDoInstruction.h
  inc/WNExpression.h
  inc/WNForInstruction.h
  inc/WNFunction.h
  inc/WNFunctionCallExpression.h
  inc/WNFunctionParam.h
  inc/WNFunctionTemplates.inl
  inc/WNIDExpression.h
  inc/WNIfInstruction.h
  inc/WNInstruction.h
  inc/WNInstructionList.h
  inc/WNLValue.h
  inc/WNMemberAccessExpression.h
  inc/WNNode.h
  inc/WNNodeHelpers.h
  inc/WNNullAllocation.h
  inc/WNParameter.h
  inc/WNPostExpression.h
  inc/WNPostUNExpression.h
  inc/WNReturn.h
  inc/WNSSExpression.h
  inc/WNScopedVaraiableListImpl.h
  inc/WNScopedVariableList.h
  inc/WNScriptFile.h
  inc/WNScriptLinkedList.h
  inc/WNScripting.h
  inc/WNScriptingAdditionalTypes.h
  inc/WNScriptingAllocations.h
  inc/WNScriptingConfig.h
  inc/WNScriptingEngine.h
  inc/WNScriptingEngineFactory.h
  inc/WNScriptingEngineImpl.h
  inc/WNScriptingEnums.h
  inc/WNScriptingErrors.h
  inc/WNScriptingFactoryInternal.h
  inc/WNScriptingInterop.h
  inc/WNScriptingInteropForward.h
  inc/WNScriptingLog.h
  inc/WNScriptingMemoryManager.h
  inc/WNStruct.h
  inc/WNStructAllocation.h
  inc/WNTypeElement.h
  inc/WNTypeManager.h
  inc/WNTypeManagerImpl.h
  inc/WNTypeNode.h
  inc/WNUnExpression.h
  inc/WNWhileInstruction.h
  src/WNArgList.cpp
  src/WNArrayAccessExpression.cpp
  src/WNArrayAllocation.cpp
  src/WNAssignment.cpp
  src/WNBinExpression.cpp
  src/WNCodeModule.cpp
  src/WNCondExpression.cpp
  src/WNConstantExpression.cpp
  src/WNDeclList.cpp
  src/WNDeclaration.cpp
  src/WNDoInstruction.cpp
  src/WNExpression.cpp
  src/WNForInstruction.cpp
  src/WNFunction.cpp
  src/WNFunctionCallExpression.cpp
  src/WNIDExpression.cpp
  src/WNIfInstruction.cpp
  src/WNInstruction.cpp
  src/WNInstructionList.cpp
  src/WNLValue.cpp
  src/WNMemberAccessExpression.cpp
  src/WNNullAllocation.cpp
  src/WNPostExpression.cpp
  src/WNPostUNExpression.cpp
  src/WNReturn.cpp
  src/WNSSExpression.cpp
  src/WNScopedVariableListImpl.cpp
  src/WNScriptASTLexer.cpp
  src/WNScriptASTParser.cpp
  src/WNScriptCastingInitialization.cpp
  src/WNScriptFile.cpp
  src/WNScriptFloatTypeInitialiization.cpp
  src/WNScriptFunctionInitialization.cpp
  src/WNScriptIntTypeInitialization.cpp
  src/WNScriptingEngineFactory.cpp
  src/WNScriptingEngineImpl.cpp
  src/WNScriptingFactoryInternal.cpp
  src/WNStringType.cpp
  src/WNStruct.cpp
  src/WNStructAllocation.cpp
  src/WNTypeManagerImp.cpp
  src/WNTypeNode.cpp
  src/WNUnExpression.cpp
  src/WNWhileInstruction.cpp
  )

set(WNScriptingLLVMComponents codegen instrumentation objcarcopts support mcjit ipo)
if (WN_SYSTEM STREQUAL "Android")
  list(APPEND WNScriptingLLVMComponents arm)
elseif (WN_SYSTEM STREQUAL "RPI")
  list(APPEND WNScriptingLLVMComponents arm)
elseif (WN_SYSTEM STREQUAL "Linux")
  list(APPEND WNScriptingLLVMComponents X86)
elseif (WN_SYSTEM STREQUAL "Windows")
  list(APPEND WNScriptingLLVMComponents X86)
endif()

llvm_map_components_to_libnames(llvm_libs ${WNScriptingLLVMComponents})

target_link_libraries(WNScripting WNFileSystem ${llvm_libs})
add_definitions(${LLVM_DEFINITIONS})

target_include_directories(WNScripting PUBLIC 
  ${CMAKE_SOURCE_DIR}/Externals/antlr
  ${CMAKE_SOURCE_DIR}/Externals/llvm/include
  ${CMAKE_BINARY_DIR}/Externals/llvm/include
  ${CMAKE_SOURCE_DIR}/Libraries)