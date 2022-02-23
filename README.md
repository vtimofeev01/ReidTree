# ReidTree
Implementation of tree search algorithm for best cosine similarity
Was made for
- no start list of vectors, they will be added dynamically
- hierarchy is made on maximum cross similarity on each level
<br>
How it works:<br>
- tree is made on fixed levels
- on each level is set the maximum similarity to make a new branch, otherwise it goes to next level
- there is a "stop" value, when the search stops
- to reduce a size of tree is set the value, when similerity reaches them vector will not be added
 
<br>
There is example:<br>

- blue/red are used by vector search
- red - the "best fit" path



![svg](https://user-images.githubusercontent.com/39636444/155349129-77fb9dda-c32c-46b8-8398-90f58d866a6b.svg)
