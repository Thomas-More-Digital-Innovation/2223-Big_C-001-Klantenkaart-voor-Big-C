<script lang="ts">
	export const prerender = true;
	import '../app.css';
</script>

<div class="flex flex-col h-screen justify-between">
	<slot />
	<footer class="footer footer-center p-4 bg-base-300 text-base-content">
		<div>
			<p>
				Copyright © {new Date().getFullYear()} -
				<a href="https://www.thomasmore.be" rel="noopener noreferrer" target="_blank" class="link"
					>Thomas More</a
				>
				-
				<a
					href="https://digitalinnovation.be"
					rel="noopener noreferrer"
					target="_blank"
					class="link">Digital Innovation</a
				>
			</p>
			<p>
				Created by <a
					href="https://jonasclaes.be"
					target="_blank"
					rel="noopener noreferrer"
					class="link">Jonas Claes</a
				>
				and
				<a
					href="https://github.com/hverschueren"
					target="_blank"
					rel="noopener noreferrer"
					class="link">Hanne Verschueren</a
				>
			</p>
		</div>
	</footer>
</div>
