
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
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
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
   0--------------------------------------------------------------------------------> X
   |
   x..............d..........+..............d..........x
   eNB1                     UE1,2                     eNB2
   |
   |
   */

  uint16_t numberOfUes = 2;
  uint16_t numberOfEnbs = 2;
  double distance = 500.0; // m
  double zValue = 1.5;
  double enbTxPowerDbm = 46.0;
  double simTime = 5;
  bool doGenerateRem = false;

  //---------------------------------------------------------------------
  // Lte configuration
  //---------------------------------------------------------------------

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true)); // ?


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));

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
  NodeContainer enbNodes;
  enbNodes.Create (numberOfEnbs);
  ueNodes.Create (numberOfUes);

  // Install Mobility Model in eNB
  Ptr<ListPositionAllocator> positionAllocator = CreateObject<ListPositionAllocator> ();
  positionAllocator->Add(Vector (0.0         , 0.0, zValue)); // eNB1
  positionAllocator->Add(Vector (2 * distance, 0.0, zValue)); // eNB2
  positionAllocator->Add(Vector (distance    , 0.0, zValue)); // ue1
  positionAllocator->Add(Vector (distance    , 0.0, zValue)); // ue2


  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAllocator);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);


  //---------------------------------------------------------------------
  // Lte net devices configuration
  //---------------------------------------------------------------------

  // Install LTE Devices in eNB and UEs
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);


  // Install the IP stack on the UEs
  InternetStackHelper internet;
  internet.Install (ueNodes);

  lteHelper->Attach(ueLteDevs.Get(0), enbLteDevs.Get(0));
  lteHelper->Attach(ueLteDevs.Get(1), enbLteDevs.Get(1));

  //---------------------------------------------------------------------
  // Applications setup
  //---------------------------------------------------------------------

  NS_LOG_LOGIC ("setting up applications");

  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueLteDevs, bearer); // activates saturation traffic generator

  // Add X2 inteface
  //lteHelper->AddX2Interface (enbNodes); // why?



  //---------------------------------------------------------------------
  // Traces configuration
  //---------------------------------------------------------------------


  // Uncomment to enable PCAP tracing
  //  p2ph.EnablePcapAll("pcap");

  lteHelper->EnableTraces(); // EnablePhyTraces, Mac, Rlc, Pdcp

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (0.110)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));

  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("StartTime", TimeValue (Seconds (0.110)));
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));

  // connect custom trace sinks for RRC connection establishment and handover notification
  //  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
  //                   MakeCallback (&NotifyConnectionEstablishedEnb));
  //  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
  //                   MakeCallback (&NotifyConnectionEstablishedUe));

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (doGenerateRem)
    {
      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("OutputFile", StringValue ("scene1.rem"));
      remHelper->SetAttribute ("XMin", DoubleValue (-distance / 2));
      remHelper->SetAttribute ("XMax", DoubleValue (2.5 * distance));
      remHelper->SetAttribute ("YMin", DoubleValue (-distance / 2));
      remHelper->SetAttribute ("YMax", DoubleValue (distance / 2));
      remHelper->SetAttribute ("Z", DoubleValue (zValue));

      remHelper->Install ();
      // simulation will stop right after the REM has been generated
    }
  else
    {
      Simulator::Stop (Seconds(simTime));
    }

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
