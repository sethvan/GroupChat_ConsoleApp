   This was a little exercise to see if I could have different terminals communicate with
 eachother as in a chat scenario. Due to it being a console app, it was most practical for 
 me to make it a group chat app instead. I only used text files to facitlitate different 
 terminals on the same computer to have a group chat. I just wanted to see how I could have 
 different users communicate in general, not networking per say. So i did not deal with 
 servers, clients or encryption. I will leave that for some other time.

   Unless there are still some major bugs I haven't yet found, if you run more than one 
session, the terminals should be able to have a group chat between them. 
I am using GCC 10.2.0 but not sure what the minimum necessary would be to run this.
 

compile command I use is: g++ -g -std=c++20 main.cpp GroupChat.cpp -pthread -Wall




