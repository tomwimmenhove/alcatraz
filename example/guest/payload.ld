SECTIONS
{
        .payload64 0 : {
                guest64 = .;
                guest.img.o
                guest64_end = .;
        }
}
