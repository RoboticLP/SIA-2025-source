/* since Arduino doesn't have a builtin way to split Strings, this exists
Usage:
int dataCount;
String* segments = splitString(inputString, '|', dataCount);
for (int j = 0; j < dataCount; j++) {
  segments[j]; // to access substrings
  // process substrings here
}
delete[] segments; // delete array to prevent memory leaks
*/
String* splitString(String input, char splitter, int &count) {
  // Count how many splits we'll have
  count = 1;
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      count++;
    }
  }
  
  // Create array to hold the substrings
  String* result = new String[count];
  
  // Split the string
  int index = 0;
  int lastPos = 0;
  
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == splitter) {
      result[index] = input.substring(lastPos, i);
      index++;
      lastPos = i + 1;
    }
  }
  
  // Add the last substring
  result[index] = input.substring(lastPos);
  
  return result;
}