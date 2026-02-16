// based on ref: https://stackoverflow.com/a/26959339/85472
function setSelectedClass() {
    // get all 'a' elements
    var a = document.getElementsByTagName('a');
    // loop through all 'a' elements
    for (var i = 0; i < a.length; i++) {
        // Remove the class 'selected' if it exists
        a[i].classList.remove('selected');
        // and now add it back if this the currently selected page
        if (a[i].href == location.href) {
            // add 'selected' classs to the element that was clicked
            a[i].classList.add('selected');
        }
    }
}

function setMenuExpanded(expanded) {
    var toggle = document.querySelector('.mobile-menu-toggle');
    if (!toggle) {
        return;
    }
    toggle.setAttribute('aria-expanded', expanded ? 'true' : 'false');
}

function closeMobileMenu() {
    document.body.classList.remove('menu-open');
    setMenuExpanded(false);
}

function toggleMobileMenu() {
    document.body.classList.toggle('menu-open');
    setMenuExpanded(document.body.classList.contains('menu-open'));
}

function setupMobileMenu() {
    var navLinks = document.querySelectorAll('aside .sideNav a');
    for (var i = 0; i < navLinks.length; i++) {
        navLinks[i].addEventListener('click', closeMobileMenu);
    }

    document.addEventListener('keydown', function (event) {
        if (event.key === 'Escape') {
            closeMobileMenu();
        }
    });

    window.addEventListener('resize', function () {
        if (window.innerWidth > 900) {
            closeMobileMenu();
        }
    });
}

document.addEventListener('DOMContentLoaded', setupMobileMenu);
