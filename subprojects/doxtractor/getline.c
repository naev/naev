

typedef struct {
   FILE  *fp;
   char  *buf;
   size_t siz;
   size_t off;
   size_t num;
} MYFILE;

static int my_getline(char **line, size_t *line_siz, MYFILE *fp)
{
   // return getline(line, line_siz, fp->fp);
   char *where = memchr(fp->buf + fp->off, '\n', fp->num - fp->off);
   if (!where) {
      if (fp->off) {
         fp->num -= fp->off;
         memmove(fp->buf, fp->buf + fp->off, fp->num);
         fp->off = 0;
      }
      while (!where && !feof(fp->fp)) {
         if (fp->num == fp->siz) {
            fp->siz = ((fp->siz * 3) | 3) + 1;
            fp->buf = realloc(fp->buf, fp->siz);
         }
         const size_t added =
            fread(fp->buf + fp->num, sizeof(char), fp->siz - fp->num, fp->fp);
         where = memchr(fp->buf + fp->num, '\n', added);
         fp->num += added;
      }
      if (!where)
         where = fp->buf + fp->num - 1;
   }
   const size_t n = where + 1 - (fp->buf + fp->off);

   if (n + 1 > *line_siz)
      *line = realloc(*line, (*line_siz = n + 1));

   memcpy(*line, fp->buf + fp->off, n);
   (*line)[n] = '\0';
   fp->off += n;
   if (!n)
      fp->buf = realloc(fp->buf, (fp->siz = 0));
   return (int) n - 1;
}
