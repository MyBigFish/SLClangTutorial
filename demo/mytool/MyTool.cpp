
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <typeinfo>


#include "clang/AST/AST.h"
#include "clang/AST/Expr.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Driver/Options.h"
#include "clang/AST/ASTContext.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ARCMigrate/ARCMT.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/FrontendPluginRegistry.h"


#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Signals.h"
#include "llvm/IR/Constant.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/CrashRecoveryContext.h"


using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::OptionCategory ONECCategory("my tool options");


class ONEASTVisitor : public RecursiveASTVisitor<ONEASTVisitor>
{
    private:
    ASTContext *context;
    bool start;
    public:
    
    void setContext(ASTContext &context)
    {
        this->context = &context;
    }
    
    bool VisitDecl(Decl *decl)
    {
        
        start = true;
        FullSourceLoc fullSourceLoc(decl->getLocation(), context->getSourceManager());

        
        if (!fullSourceLoc.isValid()) {
            return true;
        }
        
        if (fullSourceLoc.isInSystemHeader()) {
            return true;
        }
        
        if(isa<ObjCInterfaceDecl>(decl)) { // 接口声明
            
            ObjCInterfaceDecl *interDecl = (ObjCInterfaceDecl*)decl;
            string fileName = context->getSourceManager().getFilename(interDecl->getSourceRange().getBegin()).str();
            cout<<"filename:"<<fileName<<endl;
            
            string interName = interDecl->getNameAsString();
            string superClassName = interDecl->getSuperClass()->getNameAsString();
            cout<<" interfaceName:"<<interName<<" superClassName:"<<superClassName<<endl;
            
            for(ObjCList<ObjCProtocolDecl>::iterator it = interDecl->all_referenced_protocol_begin();it!=interDecl->all_referenced_protocol_end();it++){
                string protolName =  (*it)->getNameAsString();
                cout<<"  protolName:"<<protolName<<endl;
            }
            
            
        }
        
        /**
         @interface NSDictionary<Key : id<NSCopying>, Value>@end
         key,value就是paramter
         */
        if (isa<ObjCTypeParamDecl>(decl)){
            ObjCTypeParamDecl *paramDecl = (ObjCTypeParamDecl *)decl;
            cout<<"  interParamName:"<<paramDecl->getNameAsString()<<endl;
        }
        
        if(isa<ObjCImplDecl>(decl)){
            ObjCImplDecl *implDecl = (ObjCImplDecl*)decl;
            cout<<" impleName:"<<implDecl->getNameAsString()<<endl;
        }
        
        if(isa<ObjCCategoryDecl>(decl)){
            ObjCCategoryDecl *cateDecl = (ObjCCategoryDecl*)decl;
            cout<<" categoryName:"<<cateDecl->getNameAsString()<<endl;
        }
        
        if(isa<ObjCProtocolDecl>(decl)){
            ObjCProtocolDecl *protolDecl = (ObjCProtocolDecl *)decl;
            cout<<" protoName:"<<protolDecl->getNameAsString()<<endl;
        }
        
        if(isa<ObjCPropertyDecl>(decl)){
            ObjCPropertyDecl *propertyDecl = (ObjCPropertyDecl *)decl;
            cout<<"  propertyName:"<<propertyDecl->getNameAsString()<<endl;
            
            bool objcIsInstanceMethod = propertyDecl->isInstanceProperty();
            cout<<"    isInstanceProperty:"<<objcIsInstanceMethod<<endl;
            
            string attr = propertyDecl->getType().getAsString();
            cout<<"    attr:"<<attr<<endl;
            
            cout<<"    getMethodName:"<< propertyDecl->getGetterName().getAsString()<<endl;
            
            cout<<"    setterMethodName："<<propertyDecl->getSetterName().getAsString()<<endl;
            
            cout<<"    isClassProperty:"<<propertyDecl->isClassProperty()<<endl;
            
            cout<<"    isAtomic:"<<propertyDecl->isAtomic()<<endl;
        }
        
        if (isa<ObjCIvarDecl>(decl)) {
            ObjCIvarDecl *ivarDecl = (ObjCIvarDecl *)decl;
            cout<<"   IvarDeclName:"<<ivarDecl->getNameAsString()<<endl;
        }
        
        if(isa<ObjCMethodDecl>(decl)){
            ObjCMethodDecl *methodDecl = (ObjCMethodDecl *)decl;
            cout<<"   methodDeclName:"<<methodDecl->getNameAsString()<<endl;
            
            cout<<"   isinstanceMethod:"<<methodDecl->isInstanceMethod()<<endl;
            cout<<"   selectorName:"<<methodDecl->getSelector().getAsString()<<endl;
            cout<<"   returnTypeName:"<<methodDecl->getReturnType().getAsString()<<endl;
            
            for(ArrayRef<ParmVarDecl *>::iterator it = methodDecl->param_begin();it!=methodDecl->param_end();it++){
                cout<<"    param:"<<((*it)->getNameAsString())<<" paramType:"<<(*it)->getType().getAsString()<<endl;
            }
            
        }
        
        if(isa<VarDecl>(decl)) {
            cout<<"  varName:"<<((VarDecl *)decl)->getNameAsString()<<endl;
        }

        if(isa<FunctionDecl>(decl)) {
            cout<<"   funName:"<<((FunctionDecl *)decl)->getNameAsString()<<endl;
        }
        
        if(isa<EnumConstantDecl>(decl)){
            cout<<"   enumName:"<<((EnumConstantDecl *)decl)->getNameAsString()<<endl;
        }


        
        return true;
    }
    
    bool VisitStmt(Stmt *s)
    {
        
        
        if(isa<ObjCMessageExpr>(s)){
            ObjCMessageExpr *objcExpr = (ObjCMessageExpr *)s;
            cout<<"    invokeName:"<< objcExpr->getMethodDecl()->getNameAsString()<<endl;
            cout<<"      selectorName:"<<objcExpr->getSelector().getAsString()<<endl;
            cout<<"      recevierName:"<<objcExpr->getReceiverType().getAsString()<<endl;
            Selector sel = objcExpr->getMethodDecl()->getSelector();
            unsigned int count = sel.getNumArgs();
            for (unsigned int i = 0; i < count; i++) {
                IdentifierInfo  *info = sel.getIdentifierInfoForSlot(i);
                cout<<"      slot:"<<info->getName().str()<<endl;
                
            }
    
        }
        
        if(isa<IntegerLiteral>(s)){
            IntegerLiteral *intl = (IntegerLiteral *)s;
            cout<<"int:"<<intl->getType().getAsString()<<endl;
            APInt value = intl->getValue();
            cout<<"res:"<<value.toString(10, value.isNegative())<<endl;
        }
        
        if (isa<StringLiteral>(s)) {
            StringLiteral *strl = (StringLiteral *)s;
            cout<<"str:"<<strl->getString().str()<<endl;
        }

        
        
        return true;
    }
    
    
};

class ONEASTConsumer : public ASTConsumer
{
    private:
    ONEASTVisitor visitor;
    void HandleTranslationUnit(ASTContext &context)
    {
        visitor.setContext(context);
        
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
};


class ONEASTAction : public SyntaxOnlyAction
{
    public:
    // this gets called by Clang when it invokes our Plugin
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler,llvm::StringRef InFile)
    {
        return unique_ptr<ONEASTConsumer>(new ONEASTConsumer);
    }
    
};




int main(int argc, const char **argv) {
    
    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
    llvm::CrashRecoveryContext::Disable();//优化解析时间，不处理crash情况
    clang::tooling::CommonOptionsParser op(argc, argv, ONECCategory);
    clang::tooling::ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    return Tool.run(newFrontendActionFactory<ONEASTAction>().get());
    
}
