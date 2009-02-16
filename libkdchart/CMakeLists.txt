INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET( libkmm_kdchart_la_SOURCES 
KDChart.cpp 		
KDChartAreaPainter.cpp	
KDChartAxesPainter.cpp	
KDChartAxisParams.cpp	
KDChartBarPainter.cpp	
KDChartBaseSeries.cpp 
KDChartBWPainter.cpp	
KDChartCustomBox.cpp 	
KDChartDataIntern.cpp 	
KDChartHiLoPainter.cpp	
KDChartLinesPainter.cpp	
KDChartPainter.cpp	
KDChartParams.cpp	
KDChartParams_frame.cpp
KDChartParams_io.cpp	
KDChartPiePainter.cpp	
KDChartPlaneSeries.cpp 
KDChartPolarPainter.cpp	
KDChartPropertySet.cpp	
KDChartRingPainter.cpp	
KDChartSeriesCollection.cpp 
KDChartTableBase.cpp 
KDChartTextPiece.cpp	
KDChartVectorSeries.cpp 
KDChartVectorTable.cpp 
KDChartWidget.cpp 	
KDDrawText.cpp		
KDFrame.cpp 		
KDFrameProfileSection.cpp 
KDXMLTools.cpp          
KDChartEnums.cpp 
KDChartAxisParamsWrapper.cpp 
KDChartCustomBoxWrapper.cpp 
KDChartParamsWrapper.cpp 
KDChartTableDataWrapper.cpp
)

KDE3_AUTOMOC(${libkmm_kdchart_la_SOURCES})

ADD_LIBRARY(kmm_kdchart SHARED ${libkmm_kdchart_la_SOURCES})
SET_TARGET_PROPERTIES(kmm_kdchart PROPERTIES VERSION 0.0.0 SOVERSION 0)
TARGET_LINK_LIBRARIES(kmm_kdchart  ${QT_AND_KDECORE_LIBRARIES} kdeui)

########### install files ###############
INSTALL(TARGETS kmm_kdchart 
	DESTINATION lib)
