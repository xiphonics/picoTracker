// based on ref: https://stackoverflow.com/a/26959339/85472
function setSelectedClass() {
    // get all 'a' elements
    var a = document.getElementsByTagName('a');
    // loop through all 'a' elements
    for (i = 0; i < a.length; i++) {
        // Remove the class 'selected' if it exists
        a[i].classList.remove('selected');
        // and now add it back if this the currently selected page
        if (a[i].href == location.href) {
            // add 'selected' classs to the element that was clicked
            a[i].classList.add('selected');
        }
    }
}