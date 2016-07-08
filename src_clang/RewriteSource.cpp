/*
	File:   RewriteSource.cc
	Author: yunjiawu
	Desc:   Parse the source code generated AST(abstract syntax tree) by clang,
			and use clang frontend tool to rewrite ast, instert stub function 
			when function in and out.
*/


#include <sstream>
#include <fstream>

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Rewriter.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Support/raw_ostream.h"


using namespace clang;
using namespace std;

namespace 
{

/*
	ASTVisitor class, implementation visit function for different types of nodes
*/
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
public:
	MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

	/*
		Rewrite node when it is a function declaration
	*/
	bool VisitFunctionDecl(FunctionDecl *D)
	{
		if( D->hasBody() && D->isThisDeclarationADefinition() )
        {
            Stmt *Body = D->getBody();
            if(Body)
            {
                // get function name
                stringstream funcName;
                funcName << D->getNameAsString();

                SourceLocation ST;
                stringstream SSBefore;
				#ifdef RW_OBJC
				SSBefore << "\n[MemUtil logMemUsage:@\"[IN][Function]::" << funcName.str() << "\"];\n";
				#else
                SSBefore << "\nLogMessage(\"" << "[IN][Function]::" << funcName.str() << "\");\n";
				#endif
                ST = Body->getLocStart().getLocWithOffset(1);
                TheRewriter.InsertTextAfter(ST, SSBefore.str());

                stringstream SSAfter;
				#ifdef RW_OBJC
				SSAfter << "\n[MemUtil logMemUsage:@\"[OUT][Function]::" << funcName.str() << "\"];\n";
				#else
				SSAfter << "\nLogMessage(\"" << "[OUT][Function]::" << funcName.str() << "\");\n";
				#endif
                ST = Body->getLocEnd().getLocWithOffset(0);
                TheRewriter.InsertTextBefore(ST, SSAfter.str()); 
            }
        }

		return true;
	}

	/*
		Rewrite node when it is a cxx method declaration for a class
	*/
	bool VisitCXXMethodDecl(CXXMethodDecl *D)
	{
		if( D->hasBody() && D->isThisDeclarationADefinition() )
        {
            Stmt *Body = D->getBody();
            if(Body)
            {
                // get class name
                stringstream interfaceName;
                
                CXXRecordDecl* pCXXRecordDecl = D->getParent();
                if(pCXXRecordDecl)
                {
                    interfaceName << pCXXRecordDecl->getNameAsString();
                }   
                else
                {
                    interfaceName << "UNKNOW";
                }   
                
                // get method name
                stringstream methodName;
                methodName << D->getNameAsString();
                
                SourceLocation ST;
                stringstream SSBefore;
				#ifdef RW_OBJC
				SSBefore << "\n[MemUtil logMemUsage:@\"[IN]" << interfaceName.str() << "::" << methodName.str() << "\"];\n";
				#else
                SSBefore << "\nLogMessage(\"[IN]" << interfaceName.str() << "::" << methodName.str() << "\");\n";
				#endif
                ST = Body->getLocStart().getLocWithOffset(1);
                TheRewriter.InsertTextAfter(ST, SSBefore.str());
                
                stringstream SSAfter;
				#ifdef RW_OBJC
				SSAfter << "\n[MemUtil logMemUsage:@\"[OUT]" << interfaceName.str() << "::" << methodName.str() << "\"];\n";
				#else
				SSAfter << "\nLogMessage(\"[OUT]" << interfaceName.str() << "::" << methodName.str() << "\");\n";
				#endif
                ST = Body->getLocEnd().getLocWithOffset(0);
                TheRewriter.InsertTextBefore(ST, SSAfter.str());
            } // end if
        } // end if

		return true;
	}

	/*
		Rewrite node when it is a objective-c method declaration for a class
	*/
	bool VisitObjCMethodDecl(ObjCMethodDecl *D)
	{
		if( D->hasBody() && D->isThisDeclarationADefinition() )
        {       
            Stmt *Body = D->getBody();
            if(Body)
            {
                // get interface name
                stringstream interfaceName;
                ObjCInterfaceDecl* pObjCInterfaceDecl = D->getClassInterface();
                if(pObjCInterfaceDecl)
                {
                    interfaceName << pObjCInterfaceDecl->getNameAsString();
                }
                else
                {
                    interfaceName << "UNKNOW";
                }

                // get method name
                stringstream methodName;
                methodName << D->getNameAsString();

                SourceLocation ST;
                stringstream SSBefore;
                SSBefore << "\n[MemUtil logMemUsage:@\"[IN]" << interfaceName.str() << "::" << methodName.str() << "\"];\n";
                ST = Body->getLocStart().getLocWithOffset(1);
                TheRewriter.InsertTextAfter(ST, SSBefore.str());

                stringstream SSAfter;
				SSAfter << "\n[MemUtil logMemUsage:@\"[OUT]" << interfaceName.str() << "::" << methodName.str() << "\"];\n";
                ST = Body->getLocEnd().getLocWithOffset(0);
                TheRewriter.InsertTextBefore(ST, SSAfter.str());
            } // end if
        } // end if

		return true;
	}

private:
	Rewriter &TheRewriter;
};

/*
	ASTConsumer clase interface implementation
*/
class RewriteSourceConsumer : public ASTConsumer
{
public:
	RewriteSourceConsumer(Rewriter &R)
		: TheVisitor(R)
	{}

	/*
		Traverse the AST decl node and visit it
	*/
	virtual void HandleTranslationUnit(ASTContext &Context)
	{
		TheVisitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
	MyASTVisitor TheVisitor;
};

/*
	FrontendAction tool interface implementation
*/
class RewriteSourceAction : public ASTFrontendAction
{
public:
	/*
		Init Rewriter class, return the Consumer object to traversal the AST
	*/
	virtual ASTConsumer *CreateASTConsumer(
		CompilerInstance &CI, llvm::StringRef InFile)
	{
		TheRewriter = new Rewriter;
		TheRewriter->setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		TheCompilerInstance = &CI;
		//return new RewriteSourceConsumer(&Compiler.getASTContext());
		return new RewriteSourceConsumer(*TheRewriter);
    }


	/*
		After rewrite ast, get source code buffer form SourceManager and put to standard output
	*/
	void EndSourceFileAction()
    {
        if (const RewriteBuffer *RewriteBuf =
            TheRewriter->getRewriteBufferFor(TheCompilerInstance->getSourceManager().getMainFileID())) 
        {       
            #ifdef RW_OBJC
            llvm::outs() << "#import \"MemUtil.h\"\n";
            #else   
            llvm::outs() << "#include \"MemUtil.h\"\n";
            #endif  
        
            // del BOM header(<feff>) if exists
            string buf = string(RewriteBuf->begin(), RewriteBuf->end());
            if( buf.size() > 3 && buf.compare(0, 3, "\xef\xbb\xbf")==0 )
            {       
                buf.replace(0, 3, ""); 
            }       
                        
            llvm::outs() << buf; 
                        
        }       
        else    
        {       
            llvm::errs() << "No changes!!!\n";
        }       
    }

private:
	Rewriter *TheRewriter;
	CompilerInstance *TheCompilerInstance;
};

} // end namespace

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		ifstream in(argv[1]);
		istreambuf_iterator<char> beg(in), end;
		string str(beg, end);
	
		Twine Code(str.c_str());
		tooling::runToolOnCode(new RewriteSourceAction, Code, argv[1]);
    }
}


