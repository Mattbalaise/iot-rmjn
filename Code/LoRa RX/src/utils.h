

String hexToString(const String &hex)
{
  String hexClean = hex;
  hexClean.trim();
  String text = "";

  // On traite de 2 en 2 caractères
  for (unsigned int i = 0; i < hexClean.length(); i += 2)
  {
    // Vérifie qu'il reste au moins 2 caractères
    if (i + 1 >= hexClean.length())
      break;

    String hexByte = hexClean.substring(i, i + 2);
    char c = (char)strtol(hexByte.c_str(), NULL, 16);
    text += c;
  }
  return text;
}

void hexToBinary(const String &hex, uint8_t *output, size_t outlen)
{
  for (size_t i = 0; i < outlen; i++)
  {
    String byteString = hex.substring(i * 2, i * 2 + 2);
    output[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
  }
}

String convertHex(const uint8_t *data, size_t len)
{
  String hex = "";
  for (size_t i = 0; i < len; i++)
  {
    if (data[i] < 0x10)
      hex += "0"; // Ajout d'un zéro pour chaque valeur < 0x10
    hex += String(data[i], HEX);
  }
  return hex;
}