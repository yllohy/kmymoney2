lib_LTLIBRARIES = libkmm_kdchart.la

libkmm_kdchart_la_SOURCES = KDChart.cpp 		\
			KDChartAreaPainter.cpp	\
			KDChartAxesPainter.cpp	\
			KDChartAxisParams.cpp	\
			KDChartBarPainter.cpp	\
			KDChartBaseSeries.cpp \
			KDChartBWPainter.cpp	\
			KDChartCustomBox.cpp 	\
			KDChartDataIntern.cpp 	\
			KDChartHiLoPainter.cpp	\
			KDChartLinesPainter.cpp	\
			KDChartPainter.cpp	\
			KDChartParams.cpp	\
			KDChartParams_frame.cpp\
			KDChartParams_io.cpp	\
			KDChartPiePainter.cpp	\
			KDChartPlaneSeries.cpp \
			KDChartPolarPainter.cpp	\
			KDChartPropertySet.cpp	\
			KDChartRingPainter.cpp	\
			KDChartSeriesCollection.cpp \
			KDChartTableBase.cpp \
			KDChartTextPiece.cpp	\
			KDChartVectorSeries.cpp \
			KDChartVectorTable.cpp \
			KDChartWidget.cpp 	\
			KDDrawText.cpp		\
			KDFrame.cpp 		\
			KDFrameProfileSection.cpp \
			KDXMLTools.cpp          \
                        KDChartEnums.cpp \
                        KDChartAxisParamsWrapper.cpp \
                        KDChartCustomBoxWrapper.cpp \
                        KDChartParamsWrapper.cpp \
                        KDChartTableDataWrapper.cpp
                        #KDChartObjectFactory.cpp 
                        #KDChartWrapperFactory.cpp

libkmm_kdchart_la_LDFLAGS = $(all_libraries) -no-undefined
libkmm_kdchart_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

noinst_HEADERS =	KDChart.h 		\
			KDChartAreaPainter.h	\
			KDChartAxesPainter.h	\
			KDChartBarPainter.h	\
			KDChartBaseSeries.h \
			KDChartBWPainter.h	\
			KDChartDataIntern.h \
			KDChartDataRegion.h \
			KDChartHiLoPainter.h	\
			KDChartLinesPainter.h	\
			KDChartNotEnoughSpaceException.h	\
			KDChartUnknownTypeException.h \
			KDChartPainter.h	\
			KDChartPiePainter.h	\
			KDChartPlaneSeries.h \
			KDChartPolarPainter.h	\
			KDChartRingPainter.h	\
			KDChartSeriesCollection.h \
			KDChartTextPiece.h	\
			KDChartUnknownTypeException.h	\
			KDChartVectorSeries.h \
			KDDrawText.h 		\
			KDXMLTools.h            \
			KDChartTableBase.h	\
			KDChartListTable.h	\
			KDChartVectorTable.h	\
			KDChartTable.h		\
			KDChartParams.h		\
			KDFrameProfileSection.h	\
			KDChartPropertySet.h	\
			KDChartDataIntern.h	\
			KDChartDataRegion.h	\
			KDFrame.h		\
			KDChartGlobal.h		\
			KDChartWidget.h		\
			KDChartCustomBox.h	\
			KDChartEnums.h		\
			KDChartTextPiece.h	\
			KDChartAxisParams.h \
			kdchart_export.h \
                        KDChartCustomBoxWrapper.h \
                        KDChartWrapperFactory.h \
                        KDChartAxisParamsWrapper.h \
                        KDChartParamsWrapper.h \
                        KDChartTableDataWrapper.h \
                        KDChartObjectFactory.h

METASOURCES = AUTO

# INCLUDES  = $(KOFFICECORE_INCLUDES) -I$(srcdir)/.. -I$(srcdir) $(all_includes)
INCLUDES  = -I$(srcdir) $(all_includes)

