#include <gtest/gtest.h>
#include "../include/WiFiNetwork.h"

TEST(WiFiNetworkTest, ConstructorAndGetters) {
    WiFiNetwork network("MyWiFi", "00:11:22:33:44:55", 75, 6, "WPA2");
    
    EXPECT_EQ(network.ssid, "MyWiFi");
    EXPECT_EQ(network.bssid, "00:11:22:33:44:55");
    EXPECT_EQ(network.signal, 75);
    EXPECT_EQ(network.channel, 6);
    EXPECT_EQ(network.security, "WPA2");
}

TEST(WiFiNetworkTest, DetermineSecurity) {
    WiFiNetwork network1("Network1", "00:11:22:33:44:55", 80, 1, "WPA2");
    EXPECT_EQ(network1.determineSecurity(), "WPA2");
    
    WiFiNetwork network2("Network2", "00:11:22:33:44:56", 80, 1, "WEP");
    EXPECT_EQ(network2.determineSecurity(), "WEP");
    
    WiFiNetwork network3("Network3", "00:11:22:33:44:57", 80, 1, "WPA");
    EXPECT_EQ(network3.determineSecurity(), "WPA");
    
    WiFiNetwork network4("Network4", "00:11:22:33:44:58", 80, 1, "WPA3");
    EXPECT_EQ(network4.determineSecurity(), "WPA3");
    
    WiFiNetwork network5("Network5", "00:11:22:33:44:59", 80, 1, "");
    EXPECT_EQ(network5.determineSecurity(), "OPEN");
}

TEST(WiFiNetworkTest, CopyConstructor) {
    WiFiNetwork original("MyWiFi", "00:11:22:33:44:55", 75, 6, "WPA2");
    WiFiNetwork copy = original;
    
    EXPECT_EQ(copy.ssid, original.ssid);
    EXPECT_EQ(copy.bssid, original.bssid);
    EXPECT_EQ(copy.signal, original.signal);
    EXPECT_EQ(copy.channel, original.channel);
    EXPECT_EQ(copy.security, original.security);
}

TEST(WiFiNetworkTest, MoveConstructor) {
    WiFiNetwork original("MyWiFi", "00:11:22:33:44:55", 75, 6, "WPA2");
    WiFiNetwork moved = std::move(original);
    
    EXPECT_EQ(moved.ssid, "MyWiFi");
    EXPECT_EQ(moved.bssid, "00:11:22:33:44:55");
    EXPECT_EQ(moved.signal, 75);
    EXPECT_EQ(moved.channel, 6);
    EXPECT_EQ(moved.security, "WPA2");
}

TEST(WiFiNetworkTest, AssignmentOperator) {
    WiFiNetwork original("MyWiFi", "00:11:22:33:44:55", 75, 6, "WPA2");
    WiFiNetwork assigned;
    assigned = original;
    
    EXPECT_EQ(assigned.ssid, original.ssid);
    EXPECT_EQ(assigned.bssid, original.bssid);
    EXPECT_EQ(assigned.signal, original.signal);
    EXPECT_EQ(assigned.channel, original.channel);
    EXPECT_EQ(assigned.security, original.security);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
