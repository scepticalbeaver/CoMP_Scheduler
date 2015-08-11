#include <chrono>
#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scene1_F2F_constantPos_wFading");

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << std::endl;
}

void
RecvMeasurementReportCallback (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti,
                               LteRrcSap::MeasurementReport measReport)
{
  std::cout << "ue# " << imsi << "\tend# " << cellId << "\t rsrp: " << (int)measReport.measResults.rsrpResult << "\n";
}



int
main ()
{
  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_ALL | LOG_LEVEL_ALL);

  // LogComponentEnable ("LteHelper", logLevel);
  // LogComponentEnable ("EpcHelper", logLevel);
  // LogComponentEnable ("EpcEnbApplication", logLevel);
  // LogComponentEnable ("EpcSgwPgwApplication", logLevel);

  // LogComponentEnable ("LteEnbRrc", logLevel);
  // LogComponentEnable ("LteEnbNetDevice", logLevel);
  // LogComponentEnable ("LteUeRrc", logLevel);
  // LogComponentEnable ("LteUeNetDevice", logLevel);


  /*
    Network topology:
   ^ Y
   |
   |
   x..............d..........+..............d..........x----------------------> X
   eNB1                     UE1,2                     eNB2
   |
   |
   */

  uint16_t numberOfUes = 3;
  uint16_t numberOfEnbs = 4;
  double distance = 500.0; // m
  double ueZValue = 1.5;

  double ueSpeed = 0.833; // 3 kmph in mps
  //Vector ueStartPos(0.0, -2.0,  ueZValue);
  Vector ueStartPos(700.0, -4.0,  ueZValue);
  Vector ueVelocity(0.0, ueSpeed, 0.0);

  double simTime = 5;

  double enbTxPowerDbm = 43.0;
  bool doGenerateRem = false;

  //---------------------------------------------------------------------
  // Lte configuration
  //---------------------------------------------------------------------

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true)); // ?


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel")); // default
//  lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::ConstantSpectrumPropagationLossModel"));

  lteHelper->SetFadingModel("ns3::TraceFadingLossModel");
  lteHelper->SetFadingModelAttribute("TraceFilename", StringValue ("../../../fading_trace_EPA_3kmph.fad")); //EPA,ETU


  lteHelper->SetFfrAlgorithmType ("ns3::LteFrSoftAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandwidth", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandwidth", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("AllowCenterUeUseEdgeSubBand", BooleanValue (true));
  lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (20));
  lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset", UintegerValue (LteRrcSap::PdschConfigDedicated::dB3));


  lteHelper->SetSchedulerType ("ns3::FdMtFfMacScheduler");
  lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue (100));

  lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");

  /*
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth)); // in number of RBs, 25
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));

  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (macroEnbDlEarfcn)); //100
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (macroEnbDlEarfcn + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (macroEnbBandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (macroEnbBandwidth));
  */

  //   lteHelper->EnableLogComponents ();
  //   LogComponentEnable ("PfFfMacScheduler", LOG_LEVEL_ALL);

  //---------------------------------------------------------------------
  // Mobility model configuration
  //---------------------------------------------------------------------


  NodeContainer ueNodes;
  NodeContainer eNBNodes;
  eNBNodes.Create (numberOfEnbs);
  ueNodes.Create (numberOfUes);

  // Install Mobility Model in eNB
  Ptr<ListPositionAllocator> eNBPositionAllocator = CreateObject<ListPositionAllocator> ();
  eNBPositionAllocator->Add(Vector (0.0         , 0.0, ueZValue)); // eNB1, cell1
  eNBPositionAllocator->Add(Vector (2 * distance, 0.0, ueZValue)); // eNB2, cell2
  eNBPositionAllocator->Add(Vector (2 * distance, 0.0, ueZValue)); // eNB2, cell3
  eNBPositionAllocator->Add(Vector (2 * distance, 0.0, ueZValue)); // eNB2, cell4

  MobilityHelper eNBMobility;
  eNBMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  eNBMobility.SetPositionAllocator(eNBPositionAllocator);
  eNBMobility.Install(eNBNodes);

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  ueMobility.Install(ueNodes);
  for (int i = 0; i < numberOfUes; i++)
    {
      ueNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(ueStartPos);
      ueNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(ueVelocity);
    }

  //---------------------------------------------------------------------
  // LTE eNB NetDevice  configuration
  //---------------------------------------------------------------------

  NetDeviceContainer enbLteDevs;

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (0));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (0));

  lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("MaxGain"    , DoubleValue (0.0));
  lteHelper->SetEnbAntennaModelAttribute ("Beamwidth"  , DoubleValue (30));
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (/*30*/0));
  enbLteDevs.Add(lteHelper->InstallEnbDevice(eNBNodes.Get(0)));


  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm / 1));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (16));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (16));

//  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  double const mainOrientation = 180 - 30;
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (mainOrientation));
  enbLteDevs.Add(lteHelper->InstallEnbDevice(eNBNodes.Get(1)));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (8));

  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (mainOrientation + 60));
  enbLteDevs.Add(lteHelper->InstallEnbDevice(eNBNodes.Get(2)));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (0));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (0));

  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (mainOrientation + 2 * 60));
  enbLteDevs.Add(lteHelper->InstallEnbDevice(eNBNodes.Get(3)));




  //---------------------------------------------------------------------
  // Lte net devices configuration
  //---------------------------------------------------------------------

  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);


  // Install the IP stack on the UEs
  InternetStackHelper internet;
  internet.Install (ueNodes);

  lteHelper->Attach(ueLteDevs.Get(0), enbLteDevs.Get(0));
  lteHelper->Attach(ueLteDevs.Get(1), enbLteDevs.Get(1));
  lteHelper->Attach(ueLteDevs.Get(2), enbLteDevs.Get(2));

  //---------------------------------------------------------------------
  // Applications setup
  //---------------------------------------------------------------------

  NS_LOG_LOGIC ("setting up applications");

  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueLteDevs, bearer); // activates saturation traffic generator


  //---------------------------------------------------------------------
  // UE measurements configuration
  //---------------------------------------------------------------------

//  LteRrcSap::ReportConfigEutra config;
//  config.eventId = LteRrcSap::ReportConfigEutra::EVENT_A1;
//  config.threshold1.choice = LteRrcSap::ThresholdEutra::THRESHOLD_RSRP;
//  config.threshold1.range = 0;
//  config.triggerQuantity = LteRrcSap::ReportConfigEutra::RSRP;
//  config.reportInterval = LteRrcSap::ReportConfigEutra::MS480;
//  std::vector<uint8_t> measIdList;

//  NetDeviceContainer::Iterator it;
//  for (it = enbLteDevs.Begin (); it != enbLteDevs.End (); it++)
//    {
//      Ptr<NetDevice> dev = *it;
//      Ptr<LteEnbNetDevice> enbDev = dev->GetObject<LteEnbNetDevice> ();
//      Ptr<LteEnbRrc> enbRrc = enbDev->GetRrc ();
//      uint8_t measId = enbRrc->AddUeMeasReportConfig (config);
//      measIdList.push_back (measId); // remember the measId created
//      enbRrc->TraceConnect ("RecvMeasurementReport",
//                            "context",
//                            MakeCallback (&RecvMeasurementReportCallback));
//    }

  //---------------------------------------------------------------------
  // Traces configuration
  //---------------------------------------------------------------------

  // Uncomment to enable PCAP tracing
  //  p2ph.EnablePcapAll("pcap");

//  lteHelper->EnableTraces();
  lteHelper->EnableDlMacTraces();
  lteHelper->EnableDlPhyTraces();
  lteHelper->EnableDlRxPhyTraces();
  lteHelper->EnableRlcTraces();

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (0.150)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (MilliSeconds(500.0)));


   //connect custom trace sinks for RRC connection establishment and handover notification
    Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                     MakeCallback (&NotifyConnectionEstablishedEnb));

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (doGenerateRem)
    {
      double left = -distance / 3;
      double right = 2 * distance + (-1) * left;
      // 4:3
      double height = distance;
      double top = - height / 2;
      double bottom = height / 2;
      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("OutputFile", StringValue ("scene1.rem"));
      remHelper->SetAttribute ("XMin", DoubleValue (left));
      remHelper->SetAttribute ("XMax", DoubleValue (right));
      remHelper->SetAttribute ("YMin", DoubleValue (top));
      remHelper->SetAttribute ("YMax", DoubleValue (bottom));
      remHelper->SetAttribute ("Z", DoubleValue (ueZValue));
//      remHelper->SetAttribute ("XRes", UintegerValue ((right - left) / 3));
//      remHelper->SetAttribute ("YRes", UintegerValue (height / 3));


      remHelper->Install ();
      // simulation will stop right after the REM has been generated
    }

  Simulator::Stop (Seconds(simTime));


  auto startTime = std::chrono::high_resolution_clock::now();

  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();
  Simulator::Destroy ();

  auto stopTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds> (stopTime - startTime).count();
  std::cerr << "Simulation time:\t" << duration << std::endl;
  return 0;

}
