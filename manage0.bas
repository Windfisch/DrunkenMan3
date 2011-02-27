declare function xtrim (ein as string) as string
dim as string datei,temp

datei=command

open datei for input as #1
do while not eof(1)
	line input #1,temp
	temp=xtrim(temp)
	if lcase(left(temp,10 ))="#include "+chr(22) then 'wir haben ne include!
loop

function xtrim (ein as string) as string
	dim as integer temp,i
	dim as string aus,foo
	foo=rtrim(ltrim(ein))
	
	for i=1 to len(foo)
		if mid(foo,i,1)=" " or mid(foo,i,1)=chr(9) then
			if temp=0 then
				aus+=" "
				temp=1
			end if
		else
			temp=0
			aus+=mid(foo,i,1)
		end if
	next
	return aus
end function
