#!/bin/bash
# extra mailx config for smtp server
# only heirloom-mailx support smtp
mailx='/usr/bin/heirloom-mailx -S smtp=smtp://mail.cse.cuhk.edu.hk -S from=tangcp@cse.cuhk.edu.hk'
#mailx='/usr/bin/heirloom-mailx -A gmail'
#mailx='/usr/bin/bsd-mailx'

if [ "$1" == '--help' ]; then
cat <<EOF
Usage: ./mmerge.sh DATABASE FIELD_#_EMAIL EMAIL_APPENDING EMAIL_SUBJECT
Merge the varibles from the DATABASE to stdin and then send email

Mandatory arguments
  DATABASE                the tab/comma/semi-comma delimited file, each row as a recond
  FIELD_#_EMAIL           the column number (started from 1) for the email address
  EMAIL_APPENDING         the appending domain (eg. @cuhk.edu.hk) for the email address
  EMAIL_SUBJECT           a static email subject
  <<stdin>>               the content of email, except %? are replaced by corresponding column

Example:
For a regular text file db.csv consists:
	dick,1,2
	chris,3,4
and a regular text file email.template consists:
	Dear %1....
you may type this to do a email merge:
	./mmerge.sh db.csv 1 '@cuhk.edu.hk' 'Re: Result release' < email.template

This is a mail merging script by Dick Tang (tangcp@cse.cuhk.edu.hk)
EOF
exit 0;
fi;

if [ $# -lt 4 ]; then
cat <<EOF
$0: missing operand
Try '$0 --help' for more information.
EOF
exit 0;
fi;

if [ ! -f $1 ]; then echo 'ERROR: Database file was not found!'; exit -1; fi
dos2unix $1
#hidden bugs, or sent mail without text but a 'noname' attachment

read -d '' -u 0 msg
if [ -z "$msg" ]; then echo 'ERROR: No standard input was found!'; exit -1; fi
#echo "$msg"

exec 3<$1
while read -u 3 -r line_contents
do
	line_contents=`echo "$line_contents" | sed 's/[;:,]/\t/g' | sed 's/ /_/g'`
	email_to=`echo "$line_contents" | cut -f $2`
	[ -z $email_to ] && continue
	email_to=$email_to$3
	email_subject=$4
	cur=$msg
	i=1;
	for replacement in $line_contents;
	do
	#echo $replacement;
	#cur=`echo "$cur" | sed "s/%$i[^0-9]/$replacement/g"`
	cur=`echo "$cur" | perl -pe "s/%$i(?![0-9])/$replacement/g"`
	i=`expr $i + 1`
	done
	
	echo "To: $email_to";
	#echo "$cur";
	echo "$cur" | $mailx -s "$email_subject" $email_to;
done


