#Exam rank 6
make a simple chat server.
I wrote a detailed and short version of this to help students pass the exam
as it has some unspoken requirements to pass like the recv(1) or listen(backlog= 128)
which may or may not be a rumor.

I based my solution on the one provided by markveligod, however mine
is much simpler to implement and much shorter (nearly half as short)
and also fixes some errors like checking for buf[strlen() - 1] without
checking if the buffer isn't empty ...

I did not pass the exam with this solution yet.