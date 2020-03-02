Enkel binär UDP klient/server
 
Denna uppgiften är en variant av uppgift 1, men det ska användas ett BINÄRT UDP protokoll mellan klient och server. Meddelande och format är definierat i protocol.h, i repot https://github.com/patrikarlos/np_assignment2 (Länkar till en externa sida.).

När en klient ansluter skickar den ett "calcMessage" där den anger vilken version av protokollet den stödjer, (type=22, message=0, protocol=17, major_version=1,minor_version=0). 

Om servern stödjer protokollet, så svarar den med ett calcProtocol message, där den bifogar uppgiften klienten ska lösa. Om servern inte stödjer det klienten angivet så svarar den med calcMessage, type=2, message = 2, major_version=1,minor_version=0. Om klienten får ett sådant meddelande, så ska den avbryta. 

Klienten tolkar, genomför uppgiften och skickar tillbaka svaret till servern.  Kommandon är, samma som tidigare, men se hur de hanteras av protokollet i calcProtocol. Servern läser svaret från klienten, jämför med sin kontroll beräkning, skickar tillbaka OK om rätt, NOT OK om fel (mha. calcMessage). 

Eftersom vi använder UDP som transport protokoll, som är förbindelse löst och icke tillförlitligt behöver vi implementera en enkel form av fel hantering. För klientens del, betyder det att klienten måste hantera timeout. För serverns del att komma ihåg vilka jobb den delat ut, och om ett svar inte inkommit innom 10s ska jobbet tas bort och klienten räknas som försvunnen. Dvs, servern behöver inte hantera timeout på meddelande DEN skickar, men klienten måste hantera timeout på sina.

Exempel 1, klienten skickar calcMessage. Meddelandet försvinner, efter 2 sekunder, ska klienten skicka om meddelandet, detta ska repeteras 3 gånger. Efter den tredje gången ska klienten avbryta, och skriva ut ett felmeddelande som talar om vad som gick fel. 

Exempel 2, klient har fått calcProtocol uppgiften genomfört uppdraget och skickar sitt svar. Samma sak som exempel 1, det blir 3x timeout, och klienten terminerar med ett fel meddelande. Det har då gått 3x2s = 6 sekunder. Givet att den initiala kommunikationen var snabb, så kommer servern efter ca 4 sekunder, notera att den inte fått svar på just det jobbet. Skriva ut ett felmeddelande och ta bort jobbet från listan av utdelade jobb. 

Exempel 3, allt har gått bra, klienten skickar in sitt svar mha calcProtocol, men den försöker luras, och anger ett annat ID än det den fick. Servern tar emot meddelandet, och kontrollerar i listan av utdelade jobb. Noterar att klienten (IP:port) inte matchar mot den informationen som är associerad med ID't i jobb listan. Servern svarar klienten med calcMessage,  type=3, message=2. 

 

För att komma igång med uppgiften, och försöka minimera utmaningen att jobba mot potentiellt okänd mark så finns det ett git repo att utgå från https://github.com/patrikarlos/np_assignment2 (Länkar till en externa sida.). 

Din inlämning ska vara iform av en tar.gz fil, som innehåller din lösning. Lösningen ska ha ett git repo, källkod samt makefile för att bygga lösningarna (server och client). Utgå från repot listat ovan. Se till att lägga in commits i repot som har någon form av progression i koden. Dvs. när du testar, experimenterar, etc så behöver inte varje sådan vara en commit. När du kommit till en punkt att du vet vad du vill ha med i den inlämnade koden, så commitar du det, och jobbar vidare. Jag kommer -inte- examinera git delarna, MEN jag vill se att ni har gjort progression när ni utvecklade koden. Dvs, repot ska inte bara vara en clone av grund repot, och en commit "solution".  tar.gz är ett sätt att packa ihop (tar) och komprimera (gz) resultat filen. Jag antar att lösningen du har skapat finns i en katalog np_assignmentX, i den finns källkod, makefile och .git katalog (repo). För att skicka in den så packa och komprimera katalogen med följande kommando (som utförs i katalogen där np1_assignmentX finns). ; tar -zcvf <akronym>-uppgiftX.tar.gz np_assignment1.  Innan du packar ihop så kör "make clean" i katalogen för att tabort objekt och exekverbara filer (kommer bygga om lösningen så objekt filer och exekverbara tar bara plats. 

Jag kommer testa din server och klient tillsammans, men jag kommer även testa klient och server mot andra implementationer för att se att din implementation följer standarden. 
