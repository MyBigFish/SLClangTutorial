

# clang 插件入门
## 1、编译插件

#### 1.1、代码下载

 a.查看xcode对应的clang版本 https://trac.macports.org/wiki/XcodeVersionInfo

 b.官网查询对应发布版本 https://opensource.apple.com/source/clang/ 

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992229039189.png)


c.根据官网用的代码查找github查找对应分支并下载代码
https://github.com/llvm-mirror/llvm

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992366857433.png)


#### 1.2、添加插件代码

a.创建插件所在文件夹

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992372911576.png)

b.编写cmakelists文件

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992373516725.png)

c.添加插件代码

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/Screen%20Shot%202017-07-05%20at%202.56.32%20PM.png)



#### 1.3、编译代码
有两种编译方式，一种使用cmake直接编译，一种使用IDE。这里介绍下使用xcode的编译方式。

a. 生成 xcodeproj 文件

```
cd /opt
sudo mkdir llvm
sudo chown `whoami` llvm
cd llvm
export LLVM_HOME=`pwd`

git clone -b release_39 git@github.com:llvm-mirror/llvm.git llvm
git clone -b release_39 git@github.com:llvm-mirror/clang.git llvm/tools/clang
git clone -b release_39 git@github.com:llvm-mirror/clang-tools-extra.git llvm/tools/clang/tools/extra
git clone -b release_39 git@github.com:llvm-mirror/compiler-rt.git llvm/projects/compiler-rt

mkdir llvm_build
cd llvm_build
cmake -G Xcode ../llvm -DCMAKE_BUILD_TYPE:STRING=MinSizeRel
```
备注：官网教程 http://llvm.org/docs/CMake.html

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992371208217.png)

b. 选择自动生成 schmema

![](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/14992371521034.png)

c. 编译 clang，myplugin 和 libclang

![Screen Shot 2017-07-05 at 2.51.14 P](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/Screen%20Shot%202017-07-05%20at%202.51.14%20PM.png)

d. 最终效果
![Screen Shot 2017-07-05 at 2.52.00 P](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/Screen%20Shot%202017-07-05%20at%202.52.00%20PM.png)


## 2、使用插件

#### 2.1、hook xcode

- 执行以下两个命令
```
sudo mv HackedClang.xcplugin `xcode-select -print-path`/../PlugIns/Xcode3Core.ideplugin/Contents/SharedSupport/Developer/Library/Xcode/Plug-ins
```


```
sudo mv HackedBuildSystem.xcspec `xcode-select -print-path`/Platforms/iPhoneSimulator.platform/Developer/Library/Xcode/Specifications
```
如果自定义 clang 路径和展示的名称，则修改以下文件
![customClang](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/customClang.png)

a. Xcode 配置
![8B76F915-D675-4411-876E-C858A446E5](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/8B76F915-D675-4411-876E-C858A446E5C7.png)

b. 添加 other_flags
![6274A045-31D0-4514-AFDA-3F1D20AF16](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/6274A045-31D0-4514-AFDA-3F1D20AF16C3.png)


```
-Xclang -load -Xclang /opt/llvm/clangplugin/libONEExtractVCPlugin.dylib -Xclang -add-plugin -Xclang ONEExtractVCPlugin -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
```

**/opt/llvm/clangplugin/libMyPlugin.dylib**:是动态库地址
**MyPlugin**：插件名称
**/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1**:使用的c++库

##### 2.2、命令行调用 clang 插件
直接替换红框的 clang 为自己编译的 clang 即可
![0BE5799F-4440-4E5E-8CD5-186935D312D1](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/0BE5799F-4440-4E5E-8CD5-186935D312D1.png)

#### 2.2 常用 API 介绍

a. 注册插件

`static clang::FrontendPluginRegistry::Add<my::MyASTAction> 
X("MyPluginName", "MyPlugin description"); `

第一个参数是插件名称,第二个是插件描述


b. 传递参数到插件

``` 
bool MyASTAction::ParseArgs(const CompilerInstance &CI, const std::vector<std::string>& args) {
        size_t cnt = args.size();
        for(int i = 0;i < cnt;i++){
            cout<<"myparam:"<<args.at(i)<<endl;
        }
       return true;
}

```
编译的时候添加plugin对应的param就可以通过该方法将param获取到.例如在other_cflag除了上期配置外添加中添加-Xclang -plugin-arg-MyPlugin -Xclang $SRCROOT/..,就可以将srcroot的路径打印出来

- ast 解析完成调用
`void MyASTConsumer::HandleTranslationUnit(ASTContext &context){}
`
- 获取父类名称`:interfDecl->getSuperClass()->>getNameAsString()
`
- 是否是实现类：`if(isa<ObjCImplDecl>(decl)){}`

- 是否是category： `if(isa<ObjCCategoryDecl>(decl)){}`
- 是否是协议： `if(isa<ObjCProtocolDecl>(decl)){}`
- 是否是 property：`if(isa<ObjCPropertyDecl>(decl)){}`

```

是否是实例变量:objcIsInstanceMethod = propertyDecl->isInstanceProperty();
property类型(修饰符例如NSString):propertyDecl->getType().getAsString();
getter方法名称:propertyDecl->getGetterName().getAsString()
setter方法名称:propertyDecl->getSetterName().getAsString()
是否只读:propertyDecl->isReadOnly()
是否是类property:propertyDecl->isClassProperty()
是否是原子性:propertyDecl->isAtomic()

```

b. 是否是成员变量 ：`if (isa<ObjCIvarDecl>(decl)) {}`
成员变量名称:`ivarDecl->getNameAsString()`

c. 是否是参数 : `if (isa<ObjCTypeParamDecl>(decl)){}`

```
@interface NSDictionary<Key : id<NSCopying>, Value>@end
key,value就是paramter
```

- 是否是方法 : `if(isa<ObjCMethodDecl>(decl)){}`

```
是否是实例方法: methodDecl->isInstanceMethod()
selector名称: methodDecl->getSelector().getAsString()
返回值类型:methodDecl->getReturnType().getAsString()
参数:
for(ArrayRef<ParmVarDecl *>::iterator it = methodDecl->param_begin();it!=methodDecl->param_end();it++){
    cout<<"参数:"<<((*it)->getNameAsString())<<"参数类型:"<<(*it)->getType().getAsString()<<endl;
}
```

d. 是否是变量方法枚举等 `if(isa<DeclRefExpr>(s)){}`

```
声明的名称:callExpr->getDecl()->->getNameAsString()
是否是变量:isa<VarDecl>(decl)
是否是函数:isa<FunctionDecl>(decl)
是否是枚举:isa<EnumConstantDecl>(decl)

```

e. 向object-c对象发送消息  `isa<ObjCMessageExpr>(s)` 

```
调用者:objcExpr->getSelector().getAsString() 
函数本身名称: objcExpr->getSelector().getAsString() 
接受消息类型:objcExpr->getReceiverType().getAsString()

```

# 3、获取编译命令

#### 3.1、获取编译命令
`xcodebuild clean 
xcodebuild -workspace OneTravel.xcworkspace -scheme OneTravel  CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -dry-run -configuration Release| ruby onepretty
`
解释：
**CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO：**表示不需要证书编译
**-dry-run:**只获取编译命令，执行

**onepretty:** 从xcpretty中抽取的工具，移除了无用代码，添加了复制头文件软连接功能


#### 3.2、修改编译命令
##### a、针对 libtool 的命令适配，其他参数见[链接](https://clang.llvm.org/docs/ClangCommandLineReference.html#fortran-compilation-flags)
**a.移除-gmodules：** 这个命令会打包编译信息，自定义的libtool不支持，而且也没有用。
**b. 移除 -c和-o及其参数:**仅解析ast，这是无用参数
**c.添加引用c++库（如果要解析一些.cpp和.mm文件）:**
`-I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1
`
**d.替换工具：**将 clang 替换成自己的工具，如` onec -- -x` `--`后面对应对应,双横线[参见](http://clang.llvm.org/docs/LibTooling.html)

效果如下（精简版）：

```
/Users/xx/Documents/plugin/bin/bin/onec /Users/xx/Documents/conn_connection.cpp  -outdir=~/Desktop -- -x c++ -arch armv7 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS10.3.sdk -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1

```


# 4、libtool 工具开发

#### 4.1 添加代码和 CMake文件

![6AF4FDD6-BD6C-4CDF-89FB-C4368594A95A](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/6AF4FDD6-BD6C-4CDF-89FB-C4368594A95A.png)

![E88D7FA1-A055-4AE1-A218-14100B54ABD1](https://github.com/LiuShulong/SLClangTutorial/blob/master/images/E88D7FA1-A055-4AE1-A218-14100B54ABD1.png)

进入 llvm_build 文件夹清空原来的文件，重新编译代码 `cmake -G Xcode ../llvm -DCMAKE_BUILD_TYPE:STRING=MinSizeRel`

#### 4.2 一些模块的简介

一些clang组件的作用：
libclangLex：预处理、词法分析、宏处理、词元（token）、语法构造；
libclangAST：该库是用来建立、管理、遍历AST
libclangParse：语法分析，用来解析词法分析后的结果
libclangSema：语义分析
libclangCodeGen：目标相关的代码分析
libclangAnaylysis：用于进行静态分析用的
libclangRewrite：代码重新写入（不知道该怎么解释，稍后理解了再做解释）
libclangBasic：clang的基础杂项库

http://clang.llvm.org/docs/InternalsManual.html


# 5 插件和 libtool 工具简单对比

|   | 插件 | libtool |
| --- | --- | --- |
| 解析速度 | 稍快 | 稍慢 |
| 包大小 | 大 | 小 |
| 获取编译命令 | 直接使用 xcodebuild 生成的命令 | 自己拼接或改造 |
| 与 xcode 集成 | 可以 | 不可以 |



