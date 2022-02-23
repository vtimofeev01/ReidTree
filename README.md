# ReidTree
Implementation of tree search algorithm for best cosine similarity
Was made for
- no start list of vectors, they will be added dynamically
- hierarchy is made on maximum cross similarity on each level
<br>
How it works:<br>
- tree is made on fixed levels<br>
- on each level is set the maximum similarity to make a new branch, otherwise it goes to next level<br>
- there is a "stop" value, when the search stops<br>
- to reduce a size of tree is set the value, when similarity reaches them vector will not be added<br>
 
<br>
There is example:<br>

- blue/red are used by vector search<br>
- red - the "best fit" path<br>
<br><br>


![svg](https://user-images.githubusercontent.com/39636444/155349129-77fb9dda-c32c-46b8-8398-90f58d866a6b.svg)
