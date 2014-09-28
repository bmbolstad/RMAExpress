### file: Makefile.unx
### 
### This Makefile is for compiliation on a Linux machine 
### .
###
###
###


WXINCLUDE = `wx-config --cppflags`
WXLIB = `wx-config --libs` 

WXBASEINCLUDE = `wx-config --cppflags`           
WXBASELIB = `wx-config --libs base,core`         

COMPILERFLAGS =  -g -ansi -O2 -D BUFFERED -D RMA_GUI_APP
COMPILERFLAGSBASE = -g -O2 -D BUFFERED 

CC = `wx-config --cxx`

all: RMAExpress RMADataConv console


RMAExpress: RMAExpress.cpp ResidualsDataGroup.o DataGroup.o  PMProbeBatch.o expressionGroup.o rma_background3.o  rlm_anova.o BitmapSettingDialog.o PreferencesDialog.o residualimages.o RawDataVisualize.o QCStatsVisualize.o read_celfile_text.o read_celfile_xda.o read_celfile_generic.o read_rme_cdf.o
	$(CC) $(COMPILERFLAGS) RMAExpress.cpp  $(WXINCLUDE) pnorm.o weightedkerneldensity.o rma_background3.o threestep_common.o medianpolish.o linpack.o psi_fns.o matrix_functions.o rlm_anova.o expressionGroup.o PMProbeBatch.o Matrix.o BufferedMatrix.o read_cdf_xda.o fread_functions.o read_generic.o read_celfile_text.o read_celfile_xda.o read_celfile_generic.o read_rme_cdf.o DataGroup.o CDFLocMapTree.o rma_common.o qnorm.o BitmapSettingDialog.o PreferencesDialog.o ResidualsImagesDrawing.o residualimages.o ResidualsDataGroup.o axes.o boxplot.o RawDataVisualize.o QCStatsVisualize.o $(WXLIB)  -o RMAExpress

RMADataConv: RMADataConv.cpp DataGroup.o PGF_CLF_to_RME.o threestep_common.o rma_common.o PreferencesDialog.o 
	$(CC) $(COMPILERFLAGS) RMADataConv.cpp Matrix.o read_cdf_xda.o threestep_common.o rma_common.o BufferedMatrix.o PreferencesDialog.o fread_functions.o read_generic.o read_celfile_text.o read_celfile_xda.o read_celfile_generic.o  read_rme_cdf.o DataGroup.o CDFLocMapTree.o read_clf.o read_pgf.o read_ps.o read_mps.o PGF_CLF_to_RME.o $(WXINCLUDE) $(WXLIB) -o RMADataConv


ResidualsDataGroup.o: DataGroup.o ResidualsDataGroup.cpp
	$(CC) -c $(COMPILERFLAGS) ResidualsDataGroup.cpp $(WXINCLUDE) -o ResidualsDataGroup.o

DataGroup.o: DataGroup.cpp CDFLocMapTree.cpp Matrix.o BufferedMatrix.o read_cdf_xda.o read_celfile_text.o read_celfile_xda.o read_celfile_text.o read_celfile_generic.o read_rme_cdf.o
	$(CC) -c $(COMPILERFLAGS) DataGroup.cpp $(WXINCLUDE) -o DataGroup.o
	$(CC) -c $(COMPILERFLAGS) CDFLocMapTree.cpp $(WXINCLUDE) -o CDFLocMapTree.o

PMProbeBatch.o: PMProbeBatch.cpp qnorm.o medianpolish.o rlm_anova.o
	$(CC) -c $(COMPILERFLAGS) PMProbeBatch.cpp $(WXINCLUDE) -o PMProbeBatch.o

qnorm.o: Preprocess/qnorm.c rma_common.c
	$(CC) -c $(COMPILERFLAGS)  Preprocess/qnorm.c $(WXINCLUDE) -o qnorm.o	
	$(CC) -c $(COMPILERFLAGS) rma_common.c $(WXINCLUDE) -o rma_common.o

medianpolish.o:  Preprocess/medianpolish.c
	$(CC) -c $(COMPILERFLAGS)  Preprocess/medianpolish.c $(WXINCLUDE) -o medianpolish.o	

rlm_anova.o: Preprocess/rlm_anova.c Preprocess/matrix_functions.c Preprocess/psi_fns.c linpack.o
	$(CC) -c $(COMPILERFLAGS)  Preprocess/rlm_anova.c $(WXINCLUDE) -o rlm_anova.o
	$(CC) -c $(COMPILERFLAGS)  Preprocess/matrix_functions.c $(WXINCLUDE) -o matrix_functions.o
	$(CC) -c $(COMPILERFLAGS)  Preprocess/psi_fns.c  $(WXINCLUDE) -o psi_fns.o

linpack.o: Preprocess/linpack/linpack.c
	$(CC) -c $(COMPILERFLAGS)  Preprocess/linpack/linpack.c $(WXINCLUDE) -o linpack.o


expressionGroup.o: expressionGroup.cpp
	$(CC) -c $(COMPILERFLAGS) expressionGroup.cpp $(WXINCLUDE) -o expressionGroup.o
	$(CC) -c $(COMPILERFLAGS) threestep_common.c $(WXINCLUDE) -o threestep_common.o		

rma_background3.o:  Preprocess/rma_background3.c  Preprocess/pnorm.c  Preprocess/weightedkerneldensity.c
	$(CC) -c $(COMPILERFLAGS)  Preprocess/pnorm.c  $(WXINCLUDE) -o pnorm.o
	$(CC) -c $(COMPILERFLAGS)  Preprocess/weightedkerneldensity.c  $(WXINCLUDE) -o weightedkerneldensity.o
	$(CC) -c $(COMPILERFLAGS)  Preprocess/rma_background3.c $(WXINCLUDE) -o rma_background3.o	

Matrix.o: Storage/Matrix.cpp
	$(CC) -c $(COMPILERFLAGS) Storage/Matrix.cpp  $(WXINCLUDE) -o Matrix.o

PreferencesDialog.o: PreferencesDialog.cpp
	$(CC) -c $(COMPILERFLAGS) PreferencesDialog.cpp  $(WXINCLUDE) 


residualimages.o: residualimages.cpp ResidualsImagesDrawing.o
	$(CC) -c $(COMPILERFLAGS) residualimages.cpp  $(WXINCLUDE) -o residualimages.o	

ResidualsImagesDrawing.o: ResidualsImagesDrawing.cpp
	$(CC) -c $(COMPILERFLAGS) ResidualsImagesDrawing.cpp $(WXINCLUDE) -o ResidualsImagesDrawing.o

read_cdf_xda.o: Parsing/read_cdf_xda.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_cdf_xda.c $(WXINCLUDE) -o read_cdf_xda.o	

BufferedMatrix.o:  Storage/BufferedMatrix.cpp
	$(CC) -c $(COMPILERFLAGS) Storage/BufferedMatrix.cpp $(WXINCLUDE) -o BufferedMatrix.o



RawDataVisualize.o: RawDataVisualize.cpp axes.o boxplot.o
	$(CC) -c $(COMPILERFLAGS) RawDataVisualize.cpp $(WXINCLUDE) -o RawDataVisualize.o	


QCStatsVisualize.o: QCStatsVisualize.cpp axes.o boxplot.o
	$(CC) -c $(COMPILERFLAGS) QCStatsVisualize.cpp $(WXINCLUDE) -o QCStatsVisualize.o



axes.o: Graphing/axes.cpp
	$(CC) -c $(COMPILERFLAGS) Graphing/axes.cpp $(WXINCLUDE) -o axes.o


boxplot.o: Graphing/boxplot.cpp
	$(CC) -c $(COMPILERFLAGS) Graphing/boxplot.cpp $(WXINCLUDE) -o boxplot.o


BitmapSettingDialog.o: BitmapSettingDialog.cpp
	$(CC) -c $(COMPILERFLAGS) BitmapSettingDialog.cpp $(WXINCLUDE) -o BitmapSettingDialog.o


read_rme_cdf.o: Parsing/read_rme_cdf.cpp
	$(CC) -c $(COMPILERFLAGS) Parsing/read_rme_cdf.cpp $(WXINCLUDE) -o read_rme_cdf.o

read_celfile_text.o: Parsing/read_celfile_text.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_text.c $(WXINCLUDE) -o read_celfile_text.o

read_celfile_xda.o: Parsing/read_celfile_xda.c fread_functions.o
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_xda.c $(WXINCLUDE) -o read_celfile_xda.o

read_celfile_generic.o: Parsing/read_celfile_generic.c read_generic.o fread_functions.o
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_generic.c $(WXINCLUDE) -o read_celfile_generic.o

read_generic.o:  Parsing/fread_functions.c Parsing/read_generic.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_generic.c $(WXINCLUDE) -o read_generic.o

fread_functions.o:  Parsing/fread_functions.c
	$(CC) -c $(COMPILERFLAGS) Parsing/fread_functions.c $(WXINCLUDE) -o fread_functions.o    


PGF_CLF_to_RME.o: PGF_CLF_to_RME.cpp read_clf.o read_pgf.o read_ps.o read_mps.o
	$(CC) -c  $(COMPILERFLAGS) PGF_CLF_to_RME.cpp $(WXINCLUDE) -o PGF_CLF_to_RME.o


read_clf.o: Parsing/read_clf.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_clf.c $(WXINCLUDE) -o read_clf.o

read_pgf.o: Parsing/read_pgf.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_pgf.c $(WXINCLUDE) -o read_pgf.o

read_ps.o: Parsing/read_ps.cpp
	$(CC) -c $(COMPILERFLAGS) Parsing/read_ps.cpp $(WXINCLUDE) -o read_ps.o

read_mps.o: Parsing/read_mps.cpp
	$(CC) -c $(COMPILERFLAGS) Parsing/read_mps.cpp $(WXINCLUDE) -o read_mps.o

 
clean:
	rm -f *.o
	rm -f RMAExpress
	rm -f RMADataConv


cleansrc:
	rm -f *~
	rm -f Preprocess/*~
	rm -f Storage/*~
	rm -f Graphing/*~
	rm -f Parsing/*~	

src: cleansrc
	tar c Makefile* *.c* *.h *.rc COPYING *.ico *.xpm Graphing Preprocess Storage Parsing vc.proj > RMAExpress_src.tar 
	gzip RMAExpress_src.tar


console: RMAExpressConsole.cpp DataGroupBase.o MatrixBase.o PMProbeBatchBase.o  expressionGroupBase.o rma_background3Base.o read_cdf_xdaBase.o BufferedMatrixBase.o PreferencesDialogBase.o ResidualsImagesDrawingBase.o ResidualsDataGroupBase.o QCStatsVisualizeBase.o read_rme_cdfBase.o
	$(CC) $(COMPILERFLAGSBASE) RMAExpressConsole.cpp  pnormBase.o weightedkerneldensityBase.o rma_background3Base.o MatrixBase.o  rma_commonBase.o threestep_commonBase.o  expressionGroupBase.o linpackBase.o psi_fnsBase.o matrix_functionsBase.o rlm_anovaBase.o medianpolishBase.o qnormBase.o PMProbeBatchBase.o read_cdf_xdaBase.o fread_functionsBase.o read_genericBase.o read_celfile_textBase.o read_celfile_xdaBase.o read_celfile_genericBase.o read_rme_cdfBase.o BufferedMatrixBase.o PreferencesDialogBase.o DataGroupBase.o CDFLocMapTreeBase.o ResidualsImagesDrawingBase.o ResidualsDataGroupBase.o QCStatsVisualizeBase.o $(WXBASEINCLUDE) $(WXBASELIB) -o RMAExpressConsole

ResidualsDataGroupBase.o: DataGroupBase.o ResidualsDataGroup.cpp
	$(CC) -c $(COMPILERFLAGSBASE) ResidualsDataGroup.cpp $(WXBASEINCLUDE) -o ResidualsDataGroupBase.o

DataGroupBase.o: DataGroup.cpp PreferencesDialogBase.o read_celfile_textBase.o read_celfile_xdaBase.o read_celfile_textBase.o read_celfile_genericBase.o
	$(CC) -c $(COMPILERFLAGSBASE) DataGroup.cpp $(WXBASEINCLUDE) -o DataGroupBase.o
	$(CC) -c $(COMPILERFLAGSBASE) CDFLocMapTree.cpp $(WXBASEINCLUDE) -o CDFLocMapTreeBase.o

MatrixBase.o: Storage/Matrix.cpp
	$(CC) -c $(COMPILERFLAGS) Storage/Matrix.cpp  $(WXINCLUDE) -o MatrixBase.o

PMProbeBatchBase.o: PMProbeBatch.cpp qnormBase.o medianpolishBase.o rlm_anovaBase.o
	$(CC) -c $(COMPILERFLAGSBASE) PMProbeBatch.cpp $(WXBASEINCLUDE) -o PMProbeBatchBase.o

qnormBase.o: Preprocess/qnorm.c rma_common.c
	$(CC) -c $(COMPILERFLAGSBASE) Preprocess/qnorm.c $(WXBASEINCLUDE) -o qnormBase.o	
	$(CC) -c $(COMPILERFLAGSBASE) rma_common.c $(WXBASEINCLUDE) -o rma_commonBase.o

medianpolishBase.o: Preprocess/medianpolish.c
	$(CC) -c $(COMPILERFLAGSBASE) Preprocess/medianpolish.c $(WXBASEINCLUDE) -o medianpolishBase.o	

expressionGroupBase.o: expressionGroup.cpp
	$(CC) -c $(COMPILERFLAGSBASE) expressionGroup.cpp $(WXBASEINCLUDE) -o expressionGroupBase.o
	$(CC) -c $(COMPILERFLAGSBASE) threestep_common.c $(WXBASEINCLUDE) -o threestep_commonBase.o	

rma_background3Base.o: Preprocess/rma_background3.c Preprocess/pnorm.c Preprocess/weightedkerneldensity.c
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/pnorm.c  $(WXINCLUDE) -o pnormBase.o
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/weightedkerneldensity.c  $(WXINCLUDE) -o weightedkerneldensityBase.o
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/rma_background3.c $(WXINCLUDE) -o rma_background3Base.o	

read_cdf_xdaBase.o: Parsing/read_cdf_xda.c
	$(CC) -c $(COMPILERFLAGSBASE)  Parsing/read_cdf_xda.c $(WXBASEINCLUDE) -o read_cdf_xdaBase.o	

PreferencesDialogBase.o: PreferencesDialog.cpp
	$(CC) -c $(COMPILERFLAGSBASE) PreferencesDialog.cpp  $(WXBASEINCLUDE) -o PreferencesDialogBase.o

ResidualsImagesDrawingBase.o: ResidualsImagesDrawing.cpp
	$(CC) -c $(COMPILERFLAGSBASE) ResidualsImagesDrawing.cpp $(WXBASEINCLUDE) -o ResidualsImagesDrawingBase.o

BufferedMatrixBase.o:  Storage/BufferedMatrix.cpp
	$(CC) -c $(COMPILERFLAGSBASE) Storage/BufferedMatrix.cpp $(WXBASEINCLUDE) -o BufferedMatrixBase.o

rlm_anovaBase.o: Preprocess/rlm_anova.c Preprocess/matrix_functions.c Preprocess/psi_fns.c linpackBase.o
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/rlm_anova.c $(WXINCLUDE) -o rlm_anovaBase.o
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/matrix_functions.c $(WXINCLUDE) -o matrix_functionsBase.o
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/psi_fns.c  $(WXINCLUDE) -o psi_fnsBase.o

linpackBase.o: Preprocess/linpack/linpack.c
	$(CC) -c $(COMPILERFLAGSBASE)  Preprocess/linpack/linpack.c $(WXINCLUDE) -o linpackBase.o


QCStatsVisualizeBase.o: QCStatsVisualize.cpp ####axesBase.o boxplotBase.o
	$(CC) -c $(COMPILERFLAGSBASE) QCStatsVisualize.cpp $(WXINCLUDE) -o QCStatsVisualizeBase.o

read_rme_cdfBase.o: Parsing/read_rme_cdf.cpp
	$(CC) -c $(COMPILERFLAGS) Parsing/read_rme_cdf.cpp $(WXINCLUDE) -o read_rme_cdfBase.o

read_celfile_textBase.o: Parsing/read_celfile_text.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_text.c $(WXINCLUDE) -o read_celfile_textBase.o

read_celfile_xdaBase.o: Parsing/read_celfile_xda.c fread_functionsBase.o
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_xda.c $(WXINCLUDE) -o read_celfile_xdaBase.o

read_celfile_genericBase.o: Parsing/read_celfile_generic.c read_genericBase.o fread_functionsBase.o
	$(CC) -c $(COMPILERFLAGS) Parsing/read_celfile_generic.c $(WXINCLUDE) -o read_celfile_genericBase.o

read_genericBase.o:  Parsing/fread_functions.c Parsing/read_generic.c
	$(CC) -c $(COMPILERFLAGS) Parsing/read_generic.c $(WXINCLUDE) -o read_genericBase.o

fread_functionsBase.o:  Parsing/fread_functions.c
	$(CC) -c $(COMPILERFLAGS) Parsing/fread_functions.c $(WXINCLUDE) -o fread_functionsBase.o    



###axesBase.o: Graphing/axes.cpp
###	$(CC) -c $(COMPILERFLAGSBASE) Graphing/axes.cpp $(WXINCLUDE) -o axesBase.o


###boxplotBase.o: Graphing/boxplot.cpp
###	$(CC) -c $(COMPILERFLAGSBASE) Graphing/boxplot.cpp $(WXINCLUDE) -o boxplotBase.o


###BitmapSettingDialogBase.o: BitmapSettingDialog.cpp
###	$(CC) -c $(COMPILERFLAGSBASE) BitmapSettingDialog.cpp $(WXINCLUDE) -o BitmapSettingDialogBase.o


test: BufferedMatrix.cpp
	$(CC) -c $(COMPILERFLAGS) BufferedMatrix.cpp 
	$(CC) $(COMPILERFLAGS) BufferedMatrix.o -o tests/test_BufferedMatrix test_BufferedMatrix.cpp	


Dump_CDFRME: Dump_CDFRME.cpp
	$(CC) $(COMPILERFLAGSBASE) Dump_CDFRME.cpp  $(WXBASEINCLUDE) $(WXBASELIB) -o Dump_CDFRME	


MacApps: CleanMacApps RMAExpress.app RMADataConv.app

CleanMacApps:
	rm -rf RMAExpress.app RMADataConv.app
	

RMAExpress.app: macosx/RMAExpress/Info.plist RMAExpress macosx/RMAExpress/RMAExpress.icns  
	mkdir RMAExpress.app
	mkdir RMAExpress.app/Contents
	mkdir RMAExpress.app/Contents/MacOS
	mkdir RMAExpress.app/Contents/Resources
	mkdir RMAExpress.app/Contents/libs
	cp macosx/RMAExpress/Info.plist RMAExpress.app/Contents/
	echo -n 'APPL????' > RMAExpress.app/Contents/PkgInfo
	cp RMAExpress RMAExpress.app/Contents/MacOS/RMAExpress
	cp macosx/RMAExpress/RMAExpress.icns RMAExpress.app/Contents/Resources/
	cp /usr/lib/liblzma.5.dylib RMAExpress.app/Contents/libs
	install_name_tool -change /usr/lib/liblzma.5.dylib @executable_path/../libs/liblzma.5.dylib RMAExpress.app/Contents/MacOS/RMAExpress 

	
RMADataConv.app: macosx/RMADataConv/Info.plist RMADataConv macosx/RMADataConv/RMADataConv.icns  
	mkdir RMADataConv.app
	mkdir RMADataConv.app/Contents
	mkdir RMADataConv.app/Contents/MacOS
	mkdir RMADataConv.app/Contents/Resources
	mkdir RMADataConv.app/Contents/libs
	cp macosx/RMADataConv/Info.plist RMADataConv.app/Contents/
	echo -n 'APPL????' > RMADataConv.app/Contents/PkgInfo
	cp RMADataConv RMADataConv.app/Contents/MacOS/RMADataConv
	cp macosx/RMADataConv/RMADataConv.icns RMADataConv.app/Contents/Resources/
	cp /usr/lib/liblzma.5.dylib RMADataConv.app/Contents/libs
	install_name_tool -change /usr/lib/liblzma.5.dylib @executable_path/../libs/liblzma.5.dylib RMADataConv.app/Contents/MacOS/RMADataConv 