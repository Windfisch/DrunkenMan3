dim as integer success,i,cnt,j,found,anzfehler
dim as string datei,temp,datnam,ausdatei,fehlerliste

type Tlist
	datei as string
	nummer as string
end type

dim as TList liste(50)

datei=dir ("*.cpp")
cnt=0
while datei<>""
	success=0
	for i=1 to len(datei)
		select case mid(datei,i,1)
			case "0" to "9", ".": exit for
		end select
	next

'	?"namensteil:  "; left(datei,i-1)
'	?"nummernteil: "; mid(datei,i,instr(datei,".")-i)
	
	datnam=left(datei,i-1)
	
	found=0
	for j=1 to cnt
		if lcase(liste(j).datei) = lcase(datnam) then
			found=-1
			exit for
		end if
	next
	
	temp=mid(datei,i,instr(datei,".")-i)
	if found=0 then
		cnt+=1
		liste(cnt).datei=left(datei,i-1)
		liste(cnt).nummer=temp
	else
		if val(liste(j).nummer)<val(temp) then liste(j).nummer=temp
	end if
	
	datei=dir
wend

temp=""
fehlerliste=""
anzfehler=0
for i=1 to cnt
	if lcase(liste(i).datei)="main" then ausdatei=liste(i).nummer: if ausdatei="" then ausdatei="main"
	
	?"bearbeite "+liste(i).datei+liste(i).nummer+".cpp..."
	if shell ("g++ -c "+liste(i).datei+liste(i).nummer+".cpp -o "+liste(i).datei+liste(i).nummer+".o") then fehlerliste+=liste(i).datei+liste(i).nummer+".cpp, ":anzfehler+=1
	temp+=liste(i).datei+liste(i).nummer+".o "
next

if fehlerliste<>"" then
	if anzfehler=1 then ?"In der folgenden Datei ist ein Fehler aufgetreten: "+fehlerliste else ?"In den folgenden "+trim(str(anzfehler))+" Dateien sind Fehler aufgetreten: "+fehlerliste
else
	?"linke alles zu "+ausdatei+"..."
	if shell ("g++ "+temp+"-o "+ausdatei+" "+command) then ?"Fehler beim Linken!" else	?"fertig."
end if
