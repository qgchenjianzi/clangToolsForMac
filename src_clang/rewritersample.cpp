//------------------------------------------------------------------------------
// Clang rewriter sample. Demonstrates:
    //
    // * How to use RecursiveASTVisitor to find interesting AST nodes.
    // * How to use the Rewriter API to rewrite the source code.
    //
    // Currychen (qgchenjianzi@foxmail.com)
    // This code is in the public domain
    //------------------------------------------------------------------------------
    #include <cstdio>
    #include <memory>
    #include <fstream>
    #include <string>
    #include <sstream>
    #include <iostream>
    #include <unistd.h>

#include "clang/AST/DeclObjC.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace std;

string  redirectFileFolder = "/redirect/";
string  redirectFileName = "";
string  fileName = "";
// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
    public:
    MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

    bool VisitStmt(Stmt *s) {
        // Only care about If statements.
        if (isa<IfStmt>(s)) {
            IfStmt *IfStatement = cast<IfStmt>(s);
            Stmt *Then = IfStatement->getThen();

            TheRewriter.InsertText(Then->getLocStart(), "// the 'if' part\n", true,
                                   true);

            Stmt *Else = IfStatement->getElse();
            if (Else)
            TheRewriter.InsertText(Else->getLocStart(), "// the 'else' part\n",
                                   true, true);
        }
        else if(isa<CallExpr>(s)){
            CallExpr *funcStat = cast<CallExpr>(s);
            FunctionDecl *isFunc = funcStat->getDirectCallee();
            if(isFunc){
                TheRewriter.InsertText(funcStat->getLocStart(),"//the func--->"+isFunc->getNameInfo().getName().getAsString()+"() begin called!\n",true,true);
            }

        }
        else if(isa<ReturnStmt>(s)){
            ReturnStmt *returnStat = cast<ReturnStmt>(s);
            TheRewriter.InsertText(returnStat->getLocStart(),"//the return stmt\n",true,true); 
        }
        return true;
    }

    bool VisitObjCMethodDecl(ObjCMethodDecl *f){
        // Only function definitions (with bodies), not declarations.
        if (f->hasBody()) {
            Stmt *FuncBody = f->getBody();

            // Type name as string
            QualType QT = f->getReturnType();
            std::string TypeStr = QT.getAsString();

            // Function name
            //DeclarationName DeclName = f->getName();
            //std::string FuncName = DeclName.getAsString();
            std::string FuncName = f->getNameAsString();

            // Add comment before
            std::stringstream SSBefore;
            SSBefore << "/**" << "\n"
            << " *" <<" File Name: "<<fileName << "\n"
            << " *" << " Method Name: "<<FuncName << "\n" 
            << " *" <<" Returning Type: " << TypeStr << "\n"
            << " */"<<" \n";
            SourceLocation ST = f->getSourceRange().getBegin();
            TheRewriter.InsertText(ST, SSBefore.str(), true, true);

            // And after
            /* std::stringstream SSAfter;
            SSAfter << "\n// End function " << FuncName;
            ST = FuncBody->getLocEnd().getLocWithOffset(1);
            TheRewriter.InsertText(ST, SSAfter.str(), true, true);*/
        }

        return true;

    }

        bool VisitFunctionDecl(FunctionDecl *f) {
            // Only function definitions (with bodies), not declarations.
            if (f->hasBody()) {
                Stmt *FuncBody = f->getBody();

                // Type name as string
                QualType QT = f->getReturnType();
                std::string TypeStr = QT.getAsString();

                // Function name
                DeclarationName DeclName = f->getNameInfo().getName();
                std::string FuncName = DeclName.getAsString();

                // Add comment before
                std::stringstream SSBefore;
                SSBefore << "/**" << "\n"
                << " *" <<" File Name: "<<fileName << "\n"
                << " *" << " Function Name: "<<FuncName << "\n" 
                << " *" <<" Returning Type: " << TypeStr << "\n"
                << " */"<<" \n";
                SourceLocation ST = f->getSourceRange().getBegin();
                TheRewriter.InsertText(ST, SSBefore.str(), true, true);

                // And after
                /* std::stringstream SSAfter;
                SSAfter << "\n// End function " << FuncName;
                ST = FuncBody->getLocEnd().getLocWithOffset(1);
                TheRewriter.InsertText(ST, SSAfter.str(), true, true);*/
            }

            return true;
        }

    private:
        Rewriter &TheRewriter;
        };

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
    public:
    MyASTConsumer(Rewriter &R) : Visitor(R) {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
        {
             // Traverse the declaration using our AST visitor.
             if(isa<ObjCMethodDecl>(*b))
             {
                 //利用isa阻止再遍历一次oc代码 
                 //ObjCMethodDecl *oc = (ObjCMethodDecl *) (*b);
                 //Visitor.TraverseObjCMethodDecl(oc);
             }
             else
                Visitor.TraverseDecl(*b);
        }

        return true;
    }

    private:
    MyASTVisitor Visitor;
};

void getDirBuf(char *newbuf,int size)
{
    int i = 0;
    int count = 0;
    char buf[80];
    getcwd(buf,sizeof(buf));
    for(i = 0 ;i < sizeof(buf) ;i++){
        if(buf[i] == '/')
        count ++;
        if(count < 4)
        newbuf[i] = buf[i];
        else 
        break;
    }
    newbuf[i] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        llvm::errs() << "Usage: rewritersample <filename>\n";
        return 1;
    }

    fileName = argv[1];
    // CompilerInstance will hold the instance of the Clang compiler for us,
    // managing the various objects needed to run the compiler.
    CompilerInstance TheCompInst;
    TheCompInst.createDiagnostics();

    cout << "LangOptions " << endl;
    LangOptions &lo = TheCompInst.getLangOpts();
    //C++ 编译器配置
    //lo.CPlusPlus = 1;
    //OC 编译器配置
    lo.ObjC1 = 1;
    lo.ObjC2 = 1;
    //lo.AssumeSaneOperatorNew = 1;
    // Initialize target info with the default triple for our platform.

    // 指定系统编译时查找的头文件
    //StringRef *strRef = new StringRef("-I /Users/currychen/llvm/tools/clang/lib/Headers/");
    //HeaderSearchOptions &headerSearchOption = TheCompInst.getHeaderSearchOpts();
    //headerSearchOption.AddSystemHeaderPrefix(*strRef,true);

    cout << "TargetOptions " << endl;
    auto TO = std::make_shared<TargetOptions>();
    TO->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *TI =
    TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
    TheCompInst.setTarget(TI);

    cout << "FileManager "<< endl;
    TheCompInst.createFileManager();
    FileManager &FileMgr = TheCompInst.getFileManager();
    TheCompInst.createSourceManager(FileMgr);
    SourceManager &SourceMgr = TheCompInst.getSourceManager();
    TheCompInst.createPreprocessor(TU_Module);
    TheCompInst.createASTContext();

    cout << "SetSourceMgr" << endl;
    // A Rewriter helps us manage the code rewriting task.
    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SourceMgr, lo);

    // Set the main file handled by the source manager to the input file.
    const FileEntry *FileIn = FileMgr.getFile(argv[1]);
    SourceMgr.setMainFileID(
        SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
TheCompInst.getDiagnosticClient().BeginSourceFile(
    lo, &TheCompInst.getPreprocessor());


    cout << "MyASTConsumer" << endl;
    // Create an AST consumer instance which is going to get called by
    // ParseAST.
    MyASTConsumer TheConsumer(TheRewriter);

    cout << "ParseAST" << endl;
    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
             TheCompInst.getASTContext());

    cout << "Rewrite Buffer" << endl;
    // At this point the rewriter's buffer should be full with the rewritten
    // file contents.
    const RewriteBuffer *RewriteBuf =
    TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
    llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());

    //查找最后一次 出现的位置
    size_t iPos = fileName.rfind("/");
    redirectFileName = fileName.substr(iPos+1,fileName.length()-1);

    //构建文件目录
    char parentBuf[80];
    getDirBuf(parentBuf,sizeof(parentBuf));
    string nowbuf = parentBuf;
    redirectFileFolder = nowbuf + redirectFileFolder;
    redirectFileName = redirectFileFolder + redirectFileName;
    cout << "\n The file dir is : "<<redirectFileName << endl;

    ofstream ofile;
    ofile.open(redirectFileName);
    if(RewriteBuf!=NULL)
    ofile << std::string(RewriteBuf->begin(),RewriteBuf->end()) ;
    else
    cout <<"The rewrite Buffer is NULL!" << endl;
    ofile.close();
    return 0;
}
